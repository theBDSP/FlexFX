/*
  ==============================================================================

	This dir contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/


#include "PluginEditor.h"
#include "PluginProcessor.h"




//==============================================================================
FlexFXAudioProcessor::FlexFXAudioProcessor()
	:Processor(true)
{

	for (int i = 0; i < FXTypes::NUM; ++i)
	{
		juce::String name;

		switch (i)
		{
		case FlexFXAudioProcessor::None:
			name = "None";
			break;
		case FlexFXAudioProcessor::Distortion:
			name = "Distortion";
			break;
		case FlexFXAudioProcessor::BitCrush:
			name = "BitCrush";
			break;
		case FlexFXAudioProcessor::Filter:
			name = "Filter";
			break;
		case FlexFXAudioProcessor::EQ:
			name = "EQ";
			break;
		case FlexFXAudioProcessor::PitchShift:
			name = "Pitch Shift";
			break;
		case FlexFXAudioProcessor::RingMod:
			name = "Ring Mod";
			break;
		case FlexFXAudioProcessor::Chorus:
			name = "Chorus";
			break;
		case FlexFXAudioProcessor::Flanger:
			name = "Flanger";
			break;
		case FlexFXAudioProcessor::Phaser:
			name = "Phaser";
			break;
		case FlexFXAudioProcessor::Panner:
			name = "Panner";
			break;
		case FlexFXAudioProcessor::Noise:
			name = "Noise";
			break;
		default:
			break;
		}

		if (name.isNotEmpty())
		{
			FXTypeNames.add(name);
		}

	}

	for (int i = 0; i < numFXChains; ++i)
	{
		chains.add(new bdsp::dsp::ProcessorChain<float>());
		latencyAdjusters.add(new bdsp::dsp::DelayLineBase<float>());
	}

	for (int i = 0; i < numFXSlots; ++i)
	{
		currentFXs.add(nullptr);

		emptyProcessors.add(new bdsp::dsp::EmptyProcessor<float>());
		distortions.add(new bdsp::dsp::VariableDistortion<float>(&lookups));
		bitCrushes.add(new bdsp::dsp::BitCrushDistortion<float>(&lookups));
		filters.add(new bdsp::dsp::SVF_SecondOrderBiquadFilter<float>(&lookups));
		pitchShifters.add(new bdsp::dsp::StereoPitchShifter<float>());
		ringMods.add(new bdsp::dsp::RingModulation<float>(&lookups));

		ringModSources.add(new bdsp::dsp::SampleSource<float>("RingMod" + juce::String(i + 1)));
		ringModControllers.add(new bdsp::SpectrogramController(ringModSources.getLast(), &lookups, 10));
		ringMods.getLast()->setOutputSource(ringModSources.getLast());





		choruses.add(new bdsp::dsp::Chorus<float>(&lookups));
		flangers.add(new bdsp::dsp::Flanger<float>(&lookups));
		phasers.add(new bdsp::dsp::Phaser<float>(&lookups, &parameterAttributes.getFloatAttribute("Frequency").range));
		phasers.getLast()->setNumStages(BDSP_PHASER_MAX_POLES / 2);

		panners.add(new bdsp::dsp::StereoWidener<float>(&lookups));
		pannerMeterSources.add(new bdsp::dsp::SampleSource<float>("Panner" + juce::String(i + 1)));
		pannerMeterControllers.add(new bdsp::LevelMeterController<float>(pannerMeterSources.getLast()));

		panners.getLast()->setOutputSource(pannerMeterSources.getLast());


		noises.add(new bdsp::dsp::ColoredNoiseStereo<float>());


		EQs.add(new bdsp::dsp::ParametricEQ<float>(&lookups, EQNumBands - 2));
	}


	init();



	texture = juce::PNGImageFormat::loadFrom(Texture_Data::texture_png, Texture_Data::texture_pngSize);





	settings.arr.add(new bdsp::Settings::Setting("Reduce Fractions", std::function<void(int)>(), juce::StringArray({ "Don't Reduce", "Reduce" }), 1, "UI Settings"));






	//================================================================================================================================================================================================




	settings.sort({ "UI Settings", "Key Bindings" });














	wholeNote = 240 * getSampleRate() / 120; // default BPM value so we dont get garbage values on init


	initParameters();




}

FlexFXAudioProcessor::~FlexFXAudioProcessor()
{

}

juce::AudioProcessorValueTreeState::ParameterLayout FlexFXAudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout out;



	createControlParameters(out);

	createBPMParameter(out);

	out.add(std::make_unique<juce::AudioParameterBool>("RoutingID", "Routing", false)); // true - parallel  &&  false - series

	auto percentAtt = parameterAttributes.getFloatAttribute("Percent");


	bdsp::FloatParameterAttribute parallelMixAtt;
	parallelMixAtt.range = juce::NormalisableRange<float>(-1, 1);
	parallelMixAtt.valueToTextLambda = [=](float v, int)
	{

		float norm = (v + 1) / 2;
		float valA = 1 - norm;
		float valB = norm;

		return percentAtt.valueToTextLambda(valA, 0) + " A, " + percentAtt.valueToTextLambda(valB, 0) + " B";


	};

	parallelMixAtt.textToValueLambda = [=](const juce::String& s)
	{
		if (s.containsIgnoreCase("A"))
		{
			return s.retainCharacters("0123456789.").getFloatValue() / 100;
		}
		else if (s.containsIgnoreCase("B"))
		{
			return -s.retainCharacters("0123456789.").getFloatValue() / 100;
		}
		else
		{
			return s.getFloatValue() / 100;
		}
	};

	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "ParallelMixID", "Parallel Mix", 0, parallelMixAtt));


	//Chain bypass
	for (int i = 0; i < numFXChains; ++i)
	{
		out.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("Chain" + bdsp::asciiCharToJuceString(65 + i) + "BypassID", 1), "Chain " + bdsp::asciiCharToJuceString(65 + i) + " Bypass", false));
	}


	//FX type
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("FX" + juce::String(i + 1) + "TypeID", 1), "FX " + juce::String(i + 1) + " Type", FXTypeNames, 0, "FX Type"));
	}

	//FX bypass
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("FX" + juce::String(i + 1) + "BypassID", 1), "FX " + juce::String(i + 1) + " Bypass", false));
	}

	//FX mix
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "FX" + juce::String(i + 1) + "MixID", "FX " + juce::String(i + 1) + " Mix", 1, "Percent"));
	}



	//================================================================================================================================================================================================
	//Distortion
	juce::StringArray distortionTypes(
		bdsp::dsp::DistortionTypes::Drive<float>::Name,
		bdsp::dsp::DistortionTypes::Saturation<float>::Name,
		bdsp::dsp::DistortionTypes::Tape<float>::Name,
		bdsp::dsp::DistortionTypes::HardClip<float>::Name,
		bdsp::dsp::DistortionTypes::SoftClip<float>::Name,
		bdsp::dsp::DistortionTypes::Power<float>::Name,
		bdsp::dsp::DistortionTypes::Fuzz<float>::Name,
		bdsp::dsp::DistortionTypes::Bump<float>::Name,
		bdsp::dsp::DistortionTypes::Pinch<float>::Name,
		bdsp::dsp::DistortionTypes::Tube<float>::Name,
		bdsp::dsp::DistortionTypes::SinFold<float>::Name,
		bdsp::dsp::DistortionTypes::TriFold<float>::Name
	);

	for (int i = 0; i < numFXSlots; ++i)
	{
		auto DriveTypeName = "Distortion " + juce::String(i + 1) + " Type";
		auto DriveTypeID = "Distortion" + juce::String(i + 1) + "TypeID";


		out.add(std::make_unique<juce::AudioParameterChoice>(DriveTypeID, DriveTypeName, distortionTypes, 0));
	}

	auto gainAtt = parameterAttributes.getFloatAttribute("Gain");
	gainAtt.range.end = 10;
	gainAtt.range.skew = 0.5;
	//Pre Gain
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Distortion" + juce::String(i + 1) + "PreGainID", "Distortion " + juce::String(i + 1) + "  Pre Gain", 1, gainAtt));
	}


	//Amount
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Distortion" + juce::String(i + 1) + "AmountID", "Distortion " + juce::String(i + 1) + "  Amount", 0, "Percent"));
	}


	//AGC
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<juce::AudioParameterBool>("Distortion" + juce::String(i + 1) + "GainCompensationID", "Distortion " + juce::String(i + 1) + "  Gain Compensation", false));
	}

	//================================================================================================================================================================================================
	//Bit Crush


	//Depth
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "BitCrush" + juce::String(i + 1) + "DepthID", "Bit Crush " + juce::String(i + 1) + "  Depth", 0, "Bit Crush Depth"));
	}


	//Rate
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "BitCrush" + juce::String(i + 1) + "RateID", "Bit Crush " + juce::String(i + 1) + "  Rate", 1, "Bit Crush Sampling Rate"));
	}


	//================================================================================================================================================================================================
	//Filter


	//Type
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Filter" + juce::String(i + 1) + "TypeID", "Filter " + juce::String(i + 1) + "  Type", 0.5, "Filter Type"));
	}


	//Freq
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Filter" + juce::String(i + 1) + "FrequencyID", "Filter " + juce::String(i + 1) + "  Frequency", 1000, "Frequency"));
	}


	//Q
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Filter" + juce::String(i + 1) + "ResonanceID", "Filter " + juce::String(i + 1) + "  Resonance", BDSP_FILTER_DEFAULT_Q, "Filter Q"));
	}

	//================================================================================================================================================================================================
	//EQ

	auto EQGloabalAtt = parameterAttributes.getFloatAttribute("Percent");
	EQGloabalAtt.range.start = -1;

	//Gain
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "EQ" + juce::String(i + 1) + "GlobalGainID", "EQ " + juce::String(i + 1) + " Global Gain", 1, EQGloabalAtt));
	}

	//Freq
	for (int i = 0; i < numFXSlots; ++i)
	{
		float defaultFreq = 40;
		for (int j = 0; j < EQNumBands; ++j)
		{
			out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "EQ" + juce::String(i + 1) + "Band" + juce::String(j) + "FrequencyID", "EQ " + juce::String(i + 1) + "Band" + juce::String(j) + "  Frequency", defaultFreq, "Frequency"));
			defaultFreq *= 4;
		}

	}

	//Q
	for (int i = 0; i < numFXSlots; ++i)
	{
		for (int j = 0; j < EQNumBands; ++j)
		{
			out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "EQ" + juce::String(i + 1) + "Band" + juce::String(j) + "QID", "EQ " + juce::String(i + 1) + "Band" + juce::String(j) + "  Q", BDSP_FILTER_DEFAULT_Q, "Filter Q"));
		}
	}

	//Gain
	for (int i = 0; i < numFXSlots; ++i)
	{
		for (int j = 0; j < EQNumBands; ++j)
		{
			out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "EQ" + juce::String(i + 1) + "Band" + juce::String(j) + "GainID", "EQ " + juce::String(i + 1) + "Band" + juce::String(j) + "  Gain", 0, "EQ Gain"));
		}
	}




	//================================================================================================================================================================================================
	//Pitch Shift


	//L
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "PitchShift" + juce::String(i + 1) + "LeftID", "Pitch Shift" + juce::String(i + 1) + "  Left", 0, "Pitch Shift"));
	}

	//R
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "PitchShift" + juce::String(i + 1) + "RightID", "Pitch Shift" + juce::String(i + 1) + "  Right", 0, "Pitch Shift"));
	}

	//Link
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<juce::AudioParameterBool>("PitchShift" + juce::String(i + 1) + "LinkID", "Pitch Shift" + juce::String(i + 1) + "  Link", false));
	}



	//================================================================================================================================================================================================
	//RingMod

	juce::StringArray ringModSources({ "Tone","Side-Chain","Self" });
	for (int i = 0; i < numFXSlots; ++i)
	{

		out.add(std::make_unique<juce::AudioParameterChoice>("RingMod" + juce::String(i + 1) + "SourceID", "Ring Mod " + juce::String(i + 1) + " Source", ringModSources, 0));
	}

	auto ringModFreqAtt = parameterAttributes.getFloatAttribute("Frequency");

	ringModFreqAtt.range.start = 0.1;
	ringModFreqAtt.range.end = 10000;
	//Shape
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "RingMod" + juce::String(i + 1) + "ShapeID", "Ring Mod" + juce::String(i + 1) + "  Shape", 0, "LFO Shape"));
	}
	//Skew
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "RingMod" + juce::String(i + 1) + "SkewID", "Ring Mod" + juce::String(i + 1) + "  Skew", 0.5, "Percent"));
	}
	//Freq
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "RingMod" + juce::String(i + 1) + "FrequencyID", "Ring Mod" + juce::String(i + 1) + "  Frequency", 440, ringModFreqAtt));
	}





	//================================================================================================================================================================================================
	//Chorus


	//Rate
	for (int i = 0; i < numFXSlots; ++i)
	{
		createSyncRateParameters(out, "Chorus" + juce::String(i + 1) + "RateID", "Chorus " + juce::String(i + 1) + "  Rate", 0.25f, 4.0f, 1);
	}

	auto timeAtt = parameterAttributes.getFloatAttribute("ms Time");
	timeAtt.range.start = 0;
	timeAtt.range.end = BDSP_CHORUS_DEPTH_MAX_MS;

	//Depth
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Chorus" + juce::String(i + 1) + "DepthID", bdsp::asciiCharToJuceString(65 + i) + " Chrous Depth", BDSP_CHORUS_DEPTH_MAX_MS, timeAtt));
	}

	//Stereo
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Chorus" + juce::String(i + 1) + "StereoID", "Chorus " + juce::String(i + 1) + "  Stereo", 0, "Percent"));
	}

	//Voices
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<juce::AudioParameterInt>("Chorus" + juce::String(i + 1) + "VoicesID", "Chorus " + juce::String(i + 1) + "  Voices", BDSP_CHORUS_MIN_VOICES, BDSP_CHORUS_MAX_VOICES, BDSP_CHORUS_MIN_VOICES));
	}



	//================================================================================================================================================================================================
	//Flanger


	//Rate
	for (int i = 0; i < numFXSlots; ++i)
	{
		createSyncRateParameters(out, "Flanger" + juce::String(i + 1) + "RateID", "Flanger " + juce::String(i + 1) + "  Rate", 0.25f, 4.0f, 1);
	}

	timeAtt.range.start = BDSP_FLANGER_BASE_DELAY_MIN_MS;
	timeAtt.range.end = BDSP_FLANGER_BASE_DELAY_MAX_MS;

	//Base
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Flanger" + juce::String(i + 1) + "BaseID", "Flanger " + juce::String(i + 1) + "  Base", BDSP_FLANGER_BASE_DELAY_MIN_MS, timeAtt));
	}


	timeAtt.range.start = 0;
	timeAtt.range.end = BDSP_FLANGER_DELAY_CHANGE_MAX_MS;
	//Depth
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Flanger" + juce::String(i + 1) + "DepthID", "Flanger " + juce::String(i + 1) + "  Depth", BDSP_FLANGER_DELAY_CHANGE_MAX_MS, timeAtt));
	}

	//Stereo
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Flanger" + juce::String(i + 1) + "StereoID", "Flanger " + juce::String(i + 1) + "  Stereo", 0, "Percent"));
	}

	percentAtt.range.start = -0.95;
	percentAtt.range.end = -percentAtt.range.start;
	//FB
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Flanger" + juce::String(i + 1) + "FeedbackID", "Flanger " + juce::String(i + 1) + "  Feedback", 0, percentAtt));
	}



	//================================================================================================================================================================================================
	//Phaser


	//Rate
	for (int i = 0; i < numFXSlots; ++i)
	{
		createSyncRateParameters(out, "Phaser" + juce::String(i + 1) + "RateID", "Phaser " + juce::String(i + 1) + "  Rate", 0.25f, 4.0f, 1);
	}


	//Base
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Phaser" + juce::String(i + 1) + "CenterID", "Phaser " + juce::String(i + 1) + "  Center", 0.5, "Frequency"));
	}


	//Depth
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Phaser" + juce::String(i + 1) + "DepthID", "Phaser " + juce::String(i + 1) + "  Depth", 1, "Percent"));
	}

	//Stereo
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Phaser" + juce::String(i + 1) + "StereoID", "Phaser " + juce::String(i + 1) + "  Stereo", 0, "Percent"));
	}

	percentAtt.range.end = 0.95;
	//FB
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Phaser" + juce::String(i + 1) + "FeedbackID", "Phaser " + juce::String(i + 1) + "  Feedback", 0, percentAtt));
	}



	////Stages
	//for (int i = 0; i < numFXSlots; ++i)
	//{
	//	out.add(std::make_unique<juce::AudioParameterInt>("Phaser" + juce::String(i + 1) + "StageID", "Phaser " + juce::String(i + 1) + "  Stages", 1, BDSP_PHASER_MAX_POLES / 2, 1));
	//}

	//================================================================================================================================================================================================
	//Panner


	//Gain
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Panner" + juce::String(i + 1) + "GainID", "Panner " + juce::String(i + 1) + "  Gain", 1, "Gain"));
	}

	percentAtt.range.start = -1;
	percentAtt.range.end = 1;
	//Pan
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Panner" + juce::String(i + 1) + "PanID", "Panner " + juce::String(i + 1) + "  Pan", 0, percentAtt));
	}

	//Width
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Panner" + juce::String(i + 1) + "StereoWidthID", "Panner " + juce::String(i + 1) + "  Stereo Width", 0, percentAtt));
	}



	//================================================================================================================================================================================================
	//Noise

	//Dry Gain
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Noise" + juce::String(i + 1) + "DryGainID", "Noise " + juce::String(i + 1) + " Gain", 1, "Gain"));
	}
	//Gain
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Noise" + juce::String(i + 1) + "GainID", "Noise " + juce::String(i + 1) + " Gain", 1, "Gain"));
	}

	auto colorAtt = parameterAttributes.getFloatAttribute("Percent");
	colorAtt.range.start = -1;
	colorAtt.range.end = 1;

	colorAtt.valueToTextLambda = [=](float v, int n)
	{
		if (v < 0)
		{
			return percentAtt.valueToTextLambda(abs(v), n) + " Red";
		}
		else if (v > 0)
		{
			return percentAtt.valueToTextLambda(v, n) + " Purple";
		}
		else
		{
			return juce::String("White");
		}
	};


	//Color
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Noise" + juce::String(i + 1) + "ColorID", "Noise " + juce::String(i + 1) + " Color", 0, colorAtt));
	}




	//Stereo
	for (int i = 0; i < numFXSlots; ++i)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "Noise" + juce::String(i + 1) + "StereoID", "Noise " + juce::String(i + 1) + " Stereo Width", 0, "Percent"));
	}










	return out;

}

void FlexFXAudioProcessor::setFactoryPresets()
{
	//auto size = Presets_Data::namedResourceListSize;

	//for (int i = 0; i < size; ++i)
	//{
	//	auto fileName = juce::String(Presets_Data::originalFilenames[i]);

	//	auto name = Presets_Data::namedResourceList[i];
	//	int dataSize;
	//	auto data = Presets_Data::getNamedResource(name, dataSize);

	//	factoryPresets.add(bdsp::FactoryPreset(fileName, data, dataSize));
	//}


}

void FlexFXAudioProcessor::processBlockInit(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	auto mainBuffer = getBusBuffer(buffer, true, 0);
	auto sidechainBuffer = getBusBuffer(buffer, true, 1);






	juce::Array<int> chainFXNum;
	chainFXNum.resize(numFXSlots);


	for (int i = 0; i < numFXSlots; ++i)
	{
		//set current fx pointers here
		switch (FXTypes(fxTypeParams[i].get()))
		{
		case FlexFXAudioProcessor::None:
			currentFXs.set(i, emptyProcessors.getUnchecked(i));
			break;
		case FlexFXAudioProcessor::Distortion:
			currentFXs.set(i, distortions.getUnchecked(i));
			break;
		case FlexFXAudioProcessor::BitCrush:
			currentFXs.set(i, bitCrushes.getUnchecked(i));
			break;
		case FlexFXAudioProcessor::Filter:
			currentFXs.set(i, filters.getUnchecked(i));
			break;
		case FlexFXAudioProcessor::EQ:
			currentFXs.set(i, EQs.getUnchecked(i));
			break;
		case FlexFXAudioProcessor::PitchShift:
			currentFXs.set(i, pitchShifters.getUnchecked(i));
			break;
		case FlexFXAudioProcessor::RingMod:
			currentFXs.set(i, ringMods.getUnchecked(i));
			break;
		case FlexFXAudioProcessor::Chorus:
			currentFXs.set(i, choruses.getUnchecked(i));
			break;
		case FlexFXAudioProcessor::Flanger:
			currentFXs.set(i, flangers.getUnchecked(i));
			break;
		case FlexFXAudioProcessor::Phaser:
			currentFXs.set(i, phasers.getUnchecked(i));
			break;
		case FlexFXAudioProcessor::Panner:
			currentFXs.set(i, panners.getUnchecked(i));
			break;
		case FlexFXAudioProcessor::Noise:
			currentFXs.set(i, noises.getUnchecked(i));
			break;
		default:
			break;
		}





		fxLatency[i] = currentFXs.getUnchecked(i)->getLatency() * !fxBypassParams[i].get();

		//================================================================================================================================================================================================
		//Mix

		int chainNum = i / numFXPerChain;
		currentFXs.getUnchecked(i)->setMix(fxMixParams[i].get());
		currentFXs.getUnchecked(i)->setBypassed(chainBypassParams[chainNum].get() || fxBypassParams[i].get());





		//================================================================================================================================================================================================
		//Distortion

		distortions.getUnchecked(i)->setPre(distortionPreGainParams[i].get());
		distortions.getUnchecked(i)->setAmount(distortionAmountParams[i].get());
		distortions.getUnchecked(i)->isScaled = distortionAGCParams[i].get();


		//================================================================================================================================================================================================
		//Bit Crush

		bitCrushes.getUnchecked(i)->setBitDepth(bitcrushDepthParams[i].get());
		bitCrushes.getUnchecked(i)->setReductionFactor(bitcrushRateParams[i].get());

		//================================================================================================================================================================================================
		//Filter

		filters.getUnchecked(i)->setType(filterTypeParams[i].get());
		filters.getUnchecked(i)->setFrequency(filterFreqParams[i].get());
		filters.getUnchecked(i)->setQFactor(filterQParams[i].get());

		//================================================================================================================================================================================================
		//EQ

		for (int j = 0; j < EQNumBands; ++j)
		{
			EQs.getUnchecked(i)->getBand(j)->setFrequency(EQFreqParams[i][j].get());
			EQs.getUnchecked(i)->getBand(j)->setQFactor(EQBWParams[i][j].get());
			EQs.getUnchecked(i)->getBand(j)->setGain(EQGainParams[i][j].get() * EQGlobalGainParams[i].get());
		}


		//================================================================================================================================================================================================
		//Pitch Shift
		auto r = pitchShiftLinkParams[i].get() ? pitchShiftLeftParams[i].get() : pitchShiftRightParams[i].get();

		pitchShifters.getUnchecked(i)->setShiftAmount(pitchShiftLeftParams[i].get(), r);

		//================================================================================================================================================================================================
		//RingMod
		int ringSource = ringModSourceParams[i].get();


		switch (ringSource)
		{
		case 0: // Tone
		{
			ringMods.getUnchecked(i)->setSource(bdsp::dsp::RingModulation<float>::RingModSource::Tone);
			auto* gen = ringMods.getUnchecked(i)->getToneGenerator();
			gen->setShape(ringModShapeParams[i].get());
			gen->setSkew(ringModSkewParams[i].get());
			gen->setFrequency(ringModFreqParams[i].get());
			break;
		}
		case 1: // Side-Chain
			ringMods.getUnchecked(i)->setSource(bdsp::dsp::RingModulation<float>::RingModSource::Sidechain);
			ringMods.getUnchecked(i)->setSidechain(&sidechainBuffer);
			break;
		case 2: // Self
			ringMods.getUnchecked(i)->setSource(bdsp::dsp::RingModulation<float>::RingModSource::Self);
			break;
		}


		//================================================================================================================================================================================================
		//Chorus

		auto chorusRate = 1000.0f / bdsp::calculateTimeInMs(BPMFollow->getValue(), chorusRateTimeParams[i].get(), chorusRateFracParams[i].get(), chorusRateDivisionParams[i].get(), true);
		choruses.getUnchecked(i)->setDelayChangeRate(chorusRate);
		choruses.getUnchecked(i)->setDepth(chorusDepthParams[i].get());
		choruses.getUnchecked(i)->setStereoWidth(chorusStereoParams[i].get());
		choruses.getUnchecked(i)->setNumVoices(chorusVoicesParams[i].get());

		//================================================================================================================================================================================================
		//Flanger

		auto flangerRate = 1000.0f / bdsp::calculateTimeInMs(BPMFollow->getValue(), flangerRateTimeParams[i].get(), flangerRateFracParams[i].get(), flangerRateDivisionParams[i].get(), true);
		flangers.getUnchecked(i)->setDelayChangeRate(flangerRate);
		flangers.getUnchecked(i)->setBaseDelay(flangerBaseParams[i].get());
		flangers.getUnchecked(i)->setDelayChangeMax(flangerDepthParams[i].get());
		flangers.getUnchecked(i)->setStereoSpread(flangerStereoParams[i].get());
		flangers.getUnchecked(i)->setFeedback(flangerFeedbackParams[i].get());

		//================================================================================================================================================================================================
		//Phaser

		auto phaserRate = 1000.0f / bdsp::calculateTimeInMs(BPMFollow->getValue(), phaserRateTimeParams[i].get(), phaserRateFracParams[i].get(), phaserRateDivisionParams[i].get(), true);
		phasers.getUnchecked(i)->setPhaseChangeRate(phaserRate);
		phasers.getUnchecked(i)->setCenterAndDepth(phaserCenterParams[i].get(), phaserDepthParams[i].get());
		phasers.getUnchecked(i)->setStereoWidth(phaserStereoParams[i].get());
		phasers.getUnchecked(i)->setFeedback(phaserFeedbackParams[i].get());
		//	phasers.getUnchecked(i)->setNumStages(phaserStageParams[i].get());

			//================================================================================================================================================================================================
			//Panner

		panners.getUnchecked(i)->setGain(pannerGainParams[i].get());
		panners.getUnchecked(i)->setPan(pannerPanParams[i].get());
		panners.getUnchecked(i)->setWidth(pannerWidthParams[i].get());


		//================================================================================================================================================================================================
		//Noise

		noises.getUnchecked(i)->setGain(noiseGainParams[i].get());
		noises.getUnchecked(i)->setDryMix(noiseDryParams[i].get());
		noises.getUnchecked(i)->setWetMix(1);
		noises.getUnchecked(i)->setColor(noiseColorParams[i].get());
		noises.getUnchecked(i)->setStereoWidth(noiseStereoParams[i].get());





	}


	int totalLatency;
	for (int i = 0; i < numFXChains; ++i)
	{
		chains.getUnchecked(i)->clear();
		chainLatency[i] = 0;
		chainIncluded[i] = true;
	}

	if (routingParam.get()) //parallel
	{
		bool empty = bdsp::arrayMax(fxTypeParams, numFXSlots).get() == 0;
		for (int i = 0; i < numFXChains; ++i)
		{
			if (!empty)
			{

				for (int j = 0; j < numFXPerChain; ++j)
				{
					int n = i * numFXPerChain + j;
					chainLatency[i] += fxLatency[n];
					++n;
				}
				chainIncluded[i] = true; // both are always included and mixed together
			}

		}
		totalLatency = bdsp::arrayMax(chainLatency, numFXChains);

		float parallelMixNorm = (parallelMixParam.get() + 1) / 2;

		float mixA = 1 - parallelMixNorm;
		float mixB = parallelMixNorm;

		chains.getUnchecked(0)->setOutGain(mixA);
		chains.getUnchecked(1)->setOutGain(mixB);

		for (int i = 0; i < numFXChains; ++i)
		{
			for (int j = 0; j < numFXPerChain; ++j)
			{
				addFX(i, i * numFXPerChain + j);
			}
			//chains.getUnchecked(i)->setOutGain(gainAdjust);
			latencyAdjusters.getUnchecked(i)->setDelay(totalLatency - chainLatency[i]);
		}

	}
	else
	{

		chainIncluded[0] = true;
		chainIncluded[1] = false;
		chains.getFirst()->setOutGain(1);
		latencyAdjusters.getFirst()->setDelay(0);
		totalLatency = bdsp::arraySum(chainLatency, numFXChains);
		for (int i = 0; i < numFXSlots; ++i)
		{
			addFX(0, i);
		}

	}



	setLatencySamples(getBaseLatency() + totalLatency);
}

void FlexFXAudioProcessor::processSubBlock()
{

	inputSource->trackBuffer(subBlockBuffer);
	sidechainSource->trackBuffer(sideChainSubBlockBuffer);

	for (int i = 0; i < numFXChains; ++i)
	{
		//chainBuffers.getUnchecked(i)->makeCopyOf(buffer);

		auto* buff = chainBuffers.getUnchecked(i);
		if (chainIncluded[i])
		{
			buff->makeCopyOf(subBlockBuffer);
		}
		else
		{
			buff->clear();
		}


		auto block = juce::dsp::AudioBlock<float>(*buff);
		juce::dsp::ProcessContextReplacing<float> context(block);
		latencyAdjusters.getUnchecked(i)->process(context);

		chains.getUnchecked(i)->process(context);
	}




	juce::FloatVectorOperations::add(subBlockBuffer.getWritePointer(0), chainBuffers.getUnchecked(0)->getReadPointer(0), chainBuffers.getUnchecked(1)->getReadPointer(0), subBlockBuffer.getNumSamples());
	juce::FloatVectorOperations::add(subBlockBuffer.getWritePointer(1), chainBuffers.getUnchecked(0)->getReadPointer(1), chainBuffers.getUnchecked(1)->getReadPointer(1), subBlockBuffer.getNumSamples());

	for (int i = 2; i < numFXChains; ++i)
	{
		juce::FloatVectorOperations::add(subBlockBuffer.getWritePointer(0), chainBuffers.getUnchecked(i)->getReadPointer(0), subBlockBuffer.getNumSamples());
		juce::FloatVectorOperations::add(subBlockBuffer.getWritePointer(1), chainBuffers.getUnchecked(i)->getReadPointer(1), subBlockBuffer.getNumSamples());
	}

}

void FlexFXAudioProcessor::processSubBlockBypassed()
{
}

bdsp::dsp::ProcessorChain<float>* FlexFXAudioProcessor::addFX(int chainIndex, int fxIndex)
{

	return chains.getUnchecked(chainIndex)->addProcessor(currentFXs.getUnchecked(fxIndex));

}






//==============================================================================
//==============================================================================
void FlexFXAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{

	Processor::prepareToPlay(sampleRate, samplesPerBlock);
	const juce::dsp::ProcessSpec spec = { sampleRate, subBlockSpec->maximumBlockSize, 2 };


	inputSource->prepare(spec);
	sidechainSource->prepare(spec);

	chainBuffers.clear();


	for (int i = 0; i < numFXSlots; ++i)
	{

		//emptyProcessors.getUnchecked(i)->prepare(spec);
		distortions.getUnchecked(i)->prepare(spec);
		bitCrushes.getUnchecked(i)->prepare(spec);
		filters.getUnchecked(i)->prepare(spec);
		EQs.getUnchecked(i)->prepare(spec);
		pitchShifters.getUnchecked(i)->prepare(spec);
		ringMods.getUnchecked(i)->prepare(spec);

		ringModSources.getUnchecked(i)->prepare(spec);
		ringModControllers.getUnchecked(i)->prepare(spec);



		choruses.getUnchecked(i)->prepare(spec);
		flangers.getUnchecked(i)->prepare(spec);
		phasers.getUnchecked(i)->prepare(spec);
		panners.getUnchecked(i)->prepare(spec);

		pannerMeterControllers.getUnchecked(i)->prepare(spec);
		pannerMeterSources.getUnchecked(i)->prepare(spec);

		noises.getUnchecked(i)->prepare(spec);

	}

	juce::Array<int> latencyVals;
	latencyVals.add(distortions.getUnchecked(0)->getMaxLatency());
	latencyVals.add(bitCrushes.getUnchecked(0)->getMaxLatency());
	latencyVals.add(filters.getUnchecked(0)->getMaxLatency());
	latencyVals.add(pitchShifters.getUnchecked(0)->getMaxLatency());
	latencyVals.add(choruses.getUnchecked(0)->getMaxLatency());
	latencyVals.add(flangers.getUnchecked(0)->getMaxLatency());
	latencyVals.add(phasers.getUnchecked(0)->getMaxLatency());
	latencyVals.add(panners.getUnchecked(0)->getMaxLatency());

	latencyVals.add(noises.getUnchecked(0)->getMaxLatency());




	maxSingleFXLatency = bdsp::arrayMax(latencyVals);


	for (int i = 0; i < numFXChains; ++i)
	{
		chains.getUnchecked(i)->prepare(spec);
		chainBuffers.add(new juce::AudioBuffer<float>(spec.numChannels, spec.maximumBlockSize));

		latencyAdjusters.getUnchecked(i)->setMaxDelay(numFXPerChain * maxSingleFXLatency);
		latencyAdjusters.getUnchecked(i)->prepare(spec);
	}

	initParameters();



}







void FlexFXAudioProcessor::setBypass(bool bypassState) const
{
}

//==============================================================================

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new FlexFXAudioProcessor();
}

bool FlexFXAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FlexFXAudioProcessor::createEditor()
{
	return new FlexFXAudioProcessorEditor(*this);
}

//==============================================================================









void FlexFXAudioProcessor::loadParameterPointers()
{

	routingParam.setParameter(dynamic_cast<juce::AudioParameterBool*>(parameters->getParameter("RoutingID")));
	parallelMixParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("ParallelMixID")));

	for (int i = 0; i < numFXChains; ++i)
	{
		chainBypassParams[i].setParameter(dynamic_cast<juce::AudioParameterBool*>(parameters->getParameter("Chain" + bdsp::asciiCharToJuceString(65 + i) + "BypassID")));
	}

	for (int i = 0; i < numFXSlots; ++i)
	{
		fxBypassParams[i].setParameter(dynamic_cast<juce::AudioParameterBool*>(parameters->getParameter("FX" + juce::String(i + 1) + "BypassID")));
		fxTypeParams[i].setParameter(dynamic_cast<juce::AudioParameterChoice*>(parameters->getParameter("FX" + juce::String(i + 1) + "TypeID")));
		fxMixParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("FX" + juce::String(i + 1) + "MixID")));

		//================================================================================================================================================================================================
		//Distortion
		auto distTypeParam = dynamic_cast<juce::AudioParameterChoice*>(parameters->getParameter("Distortion" + juce::String(i + 1) + "TypeID"));
		distortionTypeParams[i].setParameter(distTypeParam);
		distortions.getUnchecked(i)->setParameter(distTypeParam);

		distortionPreGainParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Distortion" + juce::String(i + 1) + "PreGainID")));
		distortionAmountParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Distortion" + juce::String(i + 1) + "AmountID")));
		distortionAGCParams[i].setParameter(dynamic_cast<juce::AudioParameterBool*>(parameters->getParameter("Distortion" + juce::String(i + 1) + "GainCompensationID")));

		//================================================================================================================================================================================================
		//Bit Crush

		bitcrushDepthParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("BitCrush" + juce::String(i + 1) + "DepthID")));
		bitcrushRateParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("BitCrush" + juce::String(i + 1) + "RateID")));

		//================================================================================================================================================================================================
		//Filter

		filterTypeParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Filter" + juce::String(i + 1) + "TypeID")));
		filterFreqParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Filter" + juce::String(i + 1) + "FrequencyID")));
		filterQParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Filter" + juce::String(i + 1) + "ResonanceID")));

		//================================================================================================================================================================================================
		//EQ

		EQGlobalGainParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("EQ" + juce::String(i + 1) + "GlobalGainID")));
		for (int j = 0; j < EQNumBands; ++j)
		{
			EQFreqParams[i][j].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("EQ" + juce::String(i + 1) + "Band" + juce::String(j) + "FrequencyID")));
			EQBWParams[i][j].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("EQ" + juce::String(i + 1) + "Band" + juce::String(j) + "QID")));
			EQGainParams[i][j].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("EQ" + juce::String(i + 1) + "Band" + juce::String(j) + "GainID")));
		}


		//================================================================================================================================================================================================
		//Pitch Shift

		pitchShiftLeftParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("PitchShift" + juce::String(i + 1) + "LeftID")));
		pitchShiftRightParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("PitchShift" + juce::String(i + 1) + "RightID")));
		pitchShiftLinkParams[i].setParameter(dynamic_cast<juce::AudioParameterBool*>(parameters->getParameter("PitchShift" + juce::String(i + 1) + "LinkID")));


		//================================================================================================================================================================================================
		//Ring Mod

		ringModSourceParams[i].setParameter(dynamic_cast<juce::AudioParameterChoice*>(parameters->getParameter("RingMod" + juce::String(i + 1) + "SourceID")));
		ringModShapeParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("RingMod" + juce::String(i + 1) + "ShapeID")));
		ringModSkewParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("RingMod" + juce::String(i + 1) + "SkewID")));
		ringModFreqParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("RingMod" + juce::String(i + 1) + "FrequencyID")));

		//================================================================================================================================================================================================
		//Chorus

		chorusRateTimeParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Chorus" + juce::String(i + 1) + "RateHzRateID")));
		chorusRateFracParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Chorus" + juce::String(i + 1) + "RateFractionID")));
		chorusRateDivisionParams[i].setParameter(dynamic_cast<juce::AudioParameterChoice*>(parameters->getParameter("Chorus" + juce::String(i + 1) + "RateDivisionID")));


		chorusDepthParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Chorus" + juce::String(i + 1) + "DepthID")));
		chorusStereoParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Chorus" + juce::String(i + 1) + "StereoID")));
		chorusVoicesParams[i].setParameter(dynamic_cast<juce::AudioParameterInt*>(parameters->getParameter("Chorus" + juce::String(i + 1) + "VoicesID")));

		//================================================================================================================================================================================================
		//Flanger

		flangerRateTimeParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Flanger" + juce::String(i + 1) + "RateHzRateID")));
		flangerRateFracParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Flanger" + juce::String(i + 1) + "RateFractionID")));
		flangerRateDivisionParams[i].setParameter(dynamic_cast<juce::AudioParameterChoice*>(parameters->getParameter("Flanger" + juce::String(i + 1) + "RateDivisionID")));


		flangerBaseParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Flanger" + juce::String(i + 1) + "BaseID")));
		flangerDepthParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Flanger" + juce::String(i + 1) + "DepthID")));
		flangerStereoParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Flanger" + juce::String(i + 1) + "StereoID")));
		flangerFeedbackParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Flanger" + juce::String(i + 1) + "FeedbackID")));

		//================================================================================================================================================================================================
		//Phaser

		phaserRateTimeParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Phaser" + juce::String(i + 1) + "RateHzRateID")));
		phaserRateFracParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Phaser" + juce::String(i + 1) + "RateFractionID")));
		phaserRateDivisionParams[i].setParameter(dynamic_cast<juce::AudioParameterChoice*>(parameters->getParameter("Phaser" + juce::String(i + 1) + "RateDivisionID")));


		phaserCenterParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Phaser" + juce::String(i + 1) + "CenterID")));
		phaserDepthParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Phaser" + juce::String(i + 1) + "DepthID")));
		phaserStereoParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Phaser" + juce::String(i + 1) + "StereoID")));
		phaserFeedbackParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Phaser" + juce::String(i + 1) + "FeedbackID")));
		//	phaserStageParams[i].setParameter(dynamic_cast<juce::AudioParameterInt*>(parameters->getParameter("Phaser" + juce::String(i + 1) + "StageID")));
			//================================================================================================================================================================================================
			//Panner

		pannerGainParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Panner" + juce::String(i + 1) + "GainID")));
		pannerPanParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Panner" + juce::String(i + 1) + "PanID")));
		pannerWidthParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Panner" + juce::String(i + 1) + "StereoWidthID")));

		//================================================================================================================================================================================================
		//Noise

		noiseGainParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Noise" + juce::String(i + 1) + "GainID")));
		noiseDryParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Noise" + juce::String(i + 1) + "DryGainID")));
		noiseColorParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Noise" + juce::String(i + 1) + "ColorID")));
		noiseStereoParams[i].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("Noise" + juce::String(i + 1) + "StereoID")));

	}
}


void FlexFXAudioProcessor::initParameters()
{
	loadParameterPointers();
	getParameterValues();

}

void FlexFXAudioProcessor::getParameterValues()
{
	bdsp::Processor::getParameterValues();

	routingParam.load();
	parallelMixParam.load();

	for (int i = 0; i < numFXChains; ++i)
	{
		chainBypassParams[i].load();
	}

	for (int i = 0; i < numFXSlots; ++i)
	{

		fxBypassParams[i].load();
		fxTypeParams[i].load();
		fxMixParams[i].load();

		//================================================================================================================================================================================================
		//Distortion
		distortionPreGainParams[i].load();
		distortionAmountParams[i].load();

		distortionAGCParams[i].load();

		//================================================================================================================================================================================================
		//Bit Crush
		bitcrushDepthParams[i].load();
		bitcrushRateParams[i].load();

		//================================================================================================================================================================================================
		//Filter
		filterTypeParams[i].load();
		filterFreqParams[i].load();
		filterQParams[i].load();


		//================================================================================================================================================================================================
		// EQ

		EQGlobalGainParams[i].load();
		for (int j = 0; j < EQNumBands; ++j)
		{
			EQFreqParams[i][j].load();
			EQBWParams[i][j].load();
			EQGainParams[i][j].load();
		}


		//================================================================================================================================================================================================
		//Pitch Shift
		pitchShiftLeftParams[i].load();
		pitchShiftRightParams[i].load();
		pitchShiftLinkParams[i].load();

		//================================================================================================================================================================================================
		//Ring Mod
		ringModSourceParams[i].load();
		ringModShapeParams[i].load();
		ringModSkewParams[i].load();
		ringModFreqParams[i].load();

		//================================================================================================================================================================================================
		//Chorus
		chorusRateTimeParams[i].load();
		chorusRateFracParams[i].load();
		chorusRateDivisionParams[i].load();

		chorusDepthParams[i].load();
		chorusStereoParams[i].load();

		chorusVoicesParams[i].load();

		//================================================================================================================================================================================================
		//Flanger
		flangerRateTimeParams[i].load();
		flangerRateFracParams[i].load();
		flangerRateDivisionParams[i].load();

		flangerBaseParams[i].load();
		flangerDepthParams[i].load();
		flangerStereoParams[i].load();
		flangerFeedbackParams[i].load();

		//================================================================================================================================================================================================
		//Phaser
		phaserRateTimeParams[i].load();
		phaserRateFracParams[i].load();
		phaserRateDivisionParams[i].load();

		phaserCenterParams[i].loadNormalized();
		phaserDepthParams[i].load();
		phaserStereoParams[i].load();
		phaserFeedbackParams[i].load();


		//================================================================================================================================================================================================
		//Panner

		pannerGainParams[i].load();
		pannerPanParams[i].load();
		pannerWidthParams[i].load();



		//================================================================================================================================================================================================
		//Noise

		noiseGainParams[i].load();
		noiseDryParams[i].load();
		noiseColorParams[i].load();
		noiseStereoParams[i].load();

	}
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FlexFXAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
	bool inNum = layouts.getMainInputChannels() == 2;
	bool outNum = layouts.getMainOutputChannels() == 2;
	bool inActive = !layouts.getMainInputChannelSet().isDisabled();
	bool outActive = !layouts.getMainOutputChannelSet().isDisabled();

	bool sidechain = juce::isPositiveAndBelow(layouts.inputBuses.size(), 3);
	return inNum && outNum && inActive && outActive && sidechain;
}

#endif
