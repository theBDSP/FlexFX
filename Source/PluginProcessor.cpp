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



	chain = std::make_unique<bdsp::dsp::ProcessorChain<float>>();
	latencyAdjuster = std::make_unique <bdsp::dsp::DelayLineBase<float>>();

	chain->clear();
	FXTypeNames.clear();

	//================================================================================================================================================================================================
	//Distortion
	FXTypeNames.add("Distortion");
	distortion = std::make_unique< bdsp::dsp::VariableDistortion<float>>(&lookups);
	chain->addProcessor(distortion.get());

	//================================================================================================================================================================================================
	//Bit Crusher
	FXTypeNames.add("Bit Crusher");
	bitCrusher = std::make_unique< bdsp::dsp::BitCrushDistortion<float>>(&lookups);
	chain->addProcessor(bitCrusher.get());

	//================================================================================================================================================================================================
	//Filter
	FXTypeNames.add("Filter");
	filter = std::make_unique< bdsp::dsp::CascadedFilter<float, bdsp::dsp::BiQuadFilters::StateVariableFilter<float>, 2>>(&lookups);
	chain->addProcessor(filter.get());

	//================================================================================================================================================================================================
	//EQ
	FXTypeNames.add("EQ");
	EQ = std::make_unique< bdsp::dsp::ParametricEQ<float>>(&lookups, EQNumBands - 2);
	chain->addProcessor(EQ.get());

	//================================================================================================================================================================================================
	//Pitch Shifter
	FXTypeNames.add("Pitch Shifter");
	pitchShifter = std::make_unique< bdsp::dsp::PitchShifter<float>>();
	chain->addProcessor(pitchShifter.get());

	//================================================================================================================================================================================================
	//Ring Modulator
	FXTypeNames.add("Ring Modulator");

	ringMod = std::make_unique< bdsp::dsp::RingModulation<float>>(&lookups);
	ringModSource = std::make_unique<bdsp::dsp::SampleSource<float>>("RingMod");
	ringMod->setOutputSource(ringModSource.get());
	chain->addProcessor(ringMod.get());

	//================================================================================================================================================================================================
	//Chorus
	FXTypeNames.add("Chrous");
	chorus = std::make_unique<bdsp::dsp::Chorus<float>>(&lookups);
	chain->addProcessor(chorus.get());

	//================================================================================================================================================================================================
	//Flanger
	FXTypeNames.add("Flanger");
	flanger = std::make_unique<bdsp::dsp::Flanger<float>>(&lookups);
	chain->addProcessor(flanger.get());

	//================================================================================================================================================================================================
	//Phaser
	FXTypeNames.add("Phaser");
	phaser = std::make_unique <bdsp::dsp::Phaser<float>>(&lookups, &parameterAttributes.getFloatAttribute("Frequency").range);
	phaser->setNumStages(BDSP_PHASER_MAX_POLES / 2);
	chain->addProcessor(phaser.get());


	//================================================================================================================================================================================================
	//Panner
	FXTypeNames.add("Panner");
	panner = std::make_unique< bdsp::dsp::StereoWidener<float>>(&lookups);
	pannerMeterSource = std::make_unique<bdsp::dsp::SampleSource<float>>("Panner");
	pannerMeterController = std::make_unique<bdsp::LevelMeterController<float>>(pannerMeterSource.get());
	panner->setOutputSource(pannerMeterSource.get());
	chain->addProcessor(panner.get());

	//================================================================================================================================================================================================
	//Noise
	FXTypeNames.add("Noise");
	noise = std::make_unique< bdsp::dsp::Noise::StereoNoiseGenerator<float, bdsp::dsp::Noise::ColoredNoise<float>>>();
	chain->addProcessor(noise.get());




	init();



	//texture = juce::PNGImageFormat::loadFrom(Texture_Data::texture_png, Texture_Data::texture_pngSize);





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


	auto DriveTypeName = "Distortion Type";
	auto DriveTypeID = "DistortionTypeID";


	out.add(std::make_unique<juce::AudioParameterChoice>(DriveTypeID, DriveTypeName, distortionTypes, 0));


	auto gainAtt = parameterAttributes.getFloatAttribute("Gain");
	gainAtt.range.end = 10;
	gainAtt.range.skew = 0.5;

	//Pre Gain
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "DistortionPreGainID", "Distortion Pre Gain", 1, gainAtt));



	//Amount
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "DistortionAmountID", "Distortion Amount", 0, "Percent"));


	//AGC
	out.add(std::make_unique<juce::AudioParameterBool>("DistortionGainCompensationID", "Distortion Gain Compensation", false));

	createBypassAndMixParam(out, "Distortion", 0);

	//================================================================================================================================================================================================
	//Bit Crush
	//Depth
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "BitCrushDepthID", "Bit Crush Depth", 0, "Bit Crush Depth"));

	//Rate
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "BitCrushRateID", "Bit Crush Rate", 1, "Bit Crush Sampling Rate"));

	createBypassAndMixParam(out, "Bit Crush", 1);

	//================================================================================================================================================================================================
	//Filter

	//Type
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "FilterTypeID", "Filter Type", 0.5, "Filter Type"));


	//Freq
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "FilterFrequencyID", "Filter Frequency", 1000, "Frequency"));


	//Q
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "FilterResonanceID", "Filter Resonance", BDSP_FILTER_DEFAULT_Q, "Filter Q"));
	createBypassAndMixParam(out, "Filter", 2);

	//================================================================================================================================================================================================
	//EQ

	auto EQGloabalAtt = parameterAttributes.getFloatAttribute("Percent");
	EQGloabalAtt.range.start = -1;

	//Gain
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "EQGlobalGainID", "EQ Global Gain", 1, EQGloabalAtt));

	//Freq
	float defaultFreq = 40;
	for (int j = 0; j < EQNumBands; ++j)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "EQBand" + juce::String(j) + "FrequencyID", "EQ Band " + juce::String(j) + " Frequency", defaultFreq, "Frequency"));
		defaultFreq *= 4;
	}

	//Q
	for (int j = 0; j < EQNumBands; ++j)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "EQBand" + juce::String(j) + "QID", "EQ Band " + juce::String(j) + " Q", BDSP_FILTER_DEFAULT_Q, "Filter Q"));
	}


	//Gain

	for (int j = 0; j < EQNumBands; ++j)
	{
		out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "EQBand" + juce::String(j) + "GainID", "EQ Band " + juce::String(j + 1) + " Gain", 0, "EQ Gain"));
	}

	createBypassAndMixParam(out, "EQ", 3);

	//================================================================================================================================================================================================
	//Pitch Shift


	//L
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "PitchShiftLeftID", "Pitch ShiftLeft", 0, "Pitch Shift"));

	//R
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "PitchShiftRightID", "Pitch ShiftRight", 0, "Pitch Shift"));

	//Link
	out.add(std::make_unique<juce::AudioParameterBool>("PitchShiftLinkID", "Pitch ShiftLink", false));

	createBypassAndMixParam(out, "Pitch Shift", 4);


	//================================================================================================================================================================================================
	//RingMod

	juce::StringArray ringModSources({ "Tone","Side-Chain","Self" });

	out.add(std::make_unique<juce::AudioParameterChoice>("RingModSourceID", "Ring Mod Source", ringModSources, 0));


	auto ringModFreqAtt = parameterAttributes.getFloatAttribute("Frequency");

	ringModFreqAtt.range.start = 0.1;
	ringModFreqAtt.range.end = 10000;
	//Shape
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "RingModShapeID", "Ring ModShape", 0, "LFO Shape"));
	//Skew
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "RingModSkewID", "Ring ModSkew", 0.5, "Percent"));
	//Freq
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "RingModFrequencyID", "Ring ModFrequency", 440, ringModFreqAtt));

	createBypassAndMixParam(out, "Ring Mod", 5);

	//================================================================================================================================================================================================
	//Chorus

	//Rate
	createSyncRateParameters(out, "ChorusRateID", "Chorus Rate", 0.25f, 4.0f, 1);

	auto timeAtt = parameterAttributes.getFloatAttribute("ms Time");
	timeAtt.range.start = 0;
	timeAtt.range.end = BDSP_CHORUS_DEPTH_MAX_MS;

	//Depth
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "ChorusDepthID", "Chrous Depth", BDSP_CHORUS_DEPTH_MAX_MS, timeAtt));

	//Stereo
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "ChorusStereoID", "Chorus Stereo", 0, "Percent"));

	//Voices
	out.add(std::make_unique<juce::AudioParameterInt>("ChorusVoicesID", "Chorus Voices", BDSP_CHORUS_MIN_VOICES, BDSP_CHORUS_MAX_VOICES, BDSP_CHORUS_MIN_VOICES));

	createBypassAndMixParam(out, "Chorus", 6);


	//================================================================================================================================================================================================
	//Flanger

	//Rate
	createSyncRateParameters(out, "FlangerRateID", "Flanger Rate", 0.25f, 4.0f, 1);

	timeAtt.range.start = BDSP_FLANGER_BASE_DELAY_MIN_MS;
	timeAtt.range.end = BDSP_FLANGER_BASE_DELAY_MAX_MS;

	//Base
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "FlangerBaseID", "Flanger Base", BDSP_FLANGER_BASE_DELAY_MIN_MS, timeAtt));


	timeAtt.range.start = 0;
	timeAtt.range.end = BDSP_FLANGER_DELAY_CHANGE_MAX_MS;
	//Depth
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "FlangerDepthID", "Flanger Depth", BDSP_FLANGER_DELAY_CHANGE_MAX_MS, timeAtt));

	//Stereo
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "FlangerStereoID", "Flanger Stereo", 0, "Percent"));

	auto percentAtt = parameterAttributes.getFloatAttribute("Percent");
	percentAtt.range.start = -0.95;
	percentAtt.range.end = -percentAtt.range.start;
	//FB
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "FlangerFeedbackID", "Flanger Feedback", 0.5, percentAtt));

	createBypassAndMixParam(out, "Flanger", 7);


	//================================================================================================================================================================================================
	//Phaser


	//Rate
	createSyncRateParameters(out, "PhaserRateID", "Phaser Rate", 0.25f, 4.0f, 1);


	//Base
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "PhaserCenterID", "Phaser Center", 0.5, "Frequency"));


	//Depth
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "PhaserDepthID", "Phaser Depth", 1, "Percent"));

	//Stereo
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "PhaserStereoID", "Phaser Stereo", 0, "Percent"));

	percentAtt.range.start = 0.0;
	percentAtt.range.end = 0.95;
	//FB
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "PhaserFeedbackID", "Phaser Feedback", 0, percentAtt));

	createBypassAndMixParam(out, "Phaser", 8);

	//================================================================================================================================================================================================
	//Panner


	//Gain
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "PannerGainID", "Panner Gain", 1, "Gain"));

	percentAtt.range.start = -1;
	percentAtt.range.end = 1;
	//Pan
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "PannerPanID", "Panner Pan", 0, percentAtt));

	//Width
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "PannerStereoWidthID", "Panner Stereo Width", 0, percentAtt));

	createBypassAndMixParam(out, "Panner", 9);


	//================================================================================================================================================================================================
	//Noise

	//Dry Gain
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "NoiseDryGainID", "Noise Gain", 1, "Gain"));
	//Gain
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "NoiseGainID", "Noise Gain", 1, "Gain"));

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
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "NoiseColorID", "Noise Color", 0, colorAtt));




	//Stereo
	out.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, out, linkableControlIDs, linkableControlNames, "NoiseStereoID", "Noise Stereo Width", 0, "Percent"));

	createBypassAndMixParam(out, "Noise", 10);



	//================================================================================================================================================================================================

	fxOrderParam = std::make_unique<bdsp::OrderedListParameter>(hiddenDummyProcessor.getParameterLayout(), FXTypeNames.size(), juce::ParameterID("FXOrder"));
	hiddenDummyProcessor.orderedParams.add(fxOrderParam.get());



	return out;

}


void FlexFXAudioProcessor::createBypassAndMixParam(juce::AudioProcessorValueTreeState::ParameterLayout& layout, const juce::String& FXName, int defaultIndex)
{
	auto trimmed = FXName.removeCharacters(" ");
	//FX bypass
	layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID(trimmed + "BypassID", 1), FXName + " Bypass", true));

	//FX mix
	layout.add(std::make_unique<bdsp::ControlParameter>(&parameterAttributes, layout, linkableControlIDs, linkableControlNames, trimmed + "MixID", FXName + " Mix", 1, "Percent"));

	layout.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID(trimmed + "IndexID", 1), FXName + " Index", 0, FXTypeNames.size(), defaultIndex));

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



	chain->reorderProcessors(fxOrderParam->getOrder());

	fxLatency = 0;

	//================================================================================================================================================================================================
	//Distortion

	distortion->setPre(distortionPreGainParam.get());
	distortion->setAmount(distortionAmountParam.get());
	distortion->isScaled = distortionAGCParam.get();
	distortion->setMix(generics[0]->mixParam.get());
	distortion->setBypassed(generics[0]->bypassParam.get());

	fxLatency += distortion->getLatency() * !distortion->isBypassed();

	//================================================================================================================================================================================================
	//Bit Crush

	bitCrusher->setBitDepth(bitcrushDepthParam.get());
	bitCrusher->setReductionFactor(bitcrushRateParam.get());
	bitCrusher->setMix(generics[1]->mixParam.get());
	bitCrusher->setBypassed(generics[1]->bypassParam.get());

	fxLatency += bitCrusher->getLatency() * !bitCrusher->isBypassed();
	//================================================================================================================================================================================================
	//Filter

	filter->setType(filterTypeParam.get());
	filter->setFrequency(filterFreqParam.get());
	filter->setQFactor(filterQParam.get());

	filter->setMix(generics[2]->mixParam.get());
	filter->setBypassed(generics[2]->bypassParam.get());

	fxLatency += filter->getLatency() * !filter->isBypassed();
	//================================================================================================================================================================================================
	//EQ

	for (int j = 0; j < EQNumBands; ++j)
	{
		EQ->getBand(j)->setFrequency(EQFreqParams[j].get());
		EQ->getBand(j)->setQFactor(EQBWParams[j].get());
		EQ->getBand(j)->setGain(EQGainParams[j].get() * EQGlobalGainParam.get());
	}
	EQ->setMix(generics[3]->mixParam.get());
	EQ->setBypassed(generics[3]->bypassParam.get());

	fxLatency += EQ->getLatency() * !EQ->isBypassed();

	//================================================================================================================================================================================================
	//Pitch Shift
	auto r = pitchShiftLinkParam.get() ? pitchShiftLeftParam.get() : pitchShiftRightParam.get();

	pitchShifter->setShiftAmount(0, pitchShiftLeftParam.get());
	pitchShifter->setShiftAmount(1, r);

	pitchShifter->setMix(generics[4]->mixParam.get());
	pitchShifter->setBypassed(generics[4]->bypassParam.get());

	fxLatency += pitchShifter->getLatency() * !pitchShifter->isBypassed();

	//================================================================================================================================================================================================
	//RingMod
	int ringSource = ringModSourceParam.get();


	switch (ringSource)
	{
	case 0: // Tone
	{
		ringMod->setModSource(bdsp::dsp::RingModulation<float>::RingModSource::Tone);
		auto* gen = ringMod->getToneGenerator();
		gen->setShape(ringModShapeParam.get());
		gen->setSkew(ringModSkewParam.get());
		gen->setFrequency(ringModFreqParam.get());
		break;
	}
	case 1: // Side-Chain
		ringMod->setModSource(bdsp::dsp::RingModulation<float>::RingModSource::Sidechain);
		ringMod->setSidechain(&sidechainBuffer);
		break;
	case 2: // Self
		ringMod->setModSource(bdsp::dsp::RingModulation<float>::RingModSource::Self);
		break;
	}

	ringMod->setMix(generics[5]->mixParam.get());
	ringMod->setBypassed(generics[5]->bypassParam.get());

	fxLatency += ringMod->getLatency() * !ringMod->isBypassed();

	//================================================================================================================================================================================================
	//Chorus

	auto chorusRate = 1000.0f / bdsp::calculateTimeInMs(BPMFollow->getValue(), chorusRateTimeParam.get(), chorusRateFracParam.get(), chorusRateDivisionParam.get(), true);
	chorus->setDelayChangeRate(chorusRate);
	chorus->setDepth(chorusDepthParam.get());
	chorus->setStereoWidth(chorusStereoParam.get());
	chorus->setNumVoices(chorusVoicesParam.get());


	chorus->setMix(generics[6]->mixParam.get());
	chorus->setBypassed(generics[6]->bypassParam.get());

	fxLatency += chorus->getLatency() * !chorus->isBypassed();

	//================================================================================================================================================================================================
	//Flanger

	auto flangerRate = 1000.0f / bdsp::calculateTimeInMs(BPMFollow->getValue(), flangerRateTimeParam.get(), flangerRateFracParam.get(), flangerRateDivisionParam.get(), true);
	flanger->setDelayChangeRate(flangerRate);
	flanger->setBaseDelay(flangerBaseParam.get());
	flanger->setDelayChangeMax(flangerDepthParam.get());
	flanger->setStereoSpread(flangerStereoParam.get());
	flanger->setFeedback(flangerFeedbackParam.get());


	flanger->setMix(generics[7]->mixParam.get());
	flanger->setBypassed(generics[7]->bypassParam.get());

	fxLatency += flanger->getLatency() * !flanger->isBypassed();

	//================================================================================================================================================================================================
	//Phaser

	auto phaserRate = 1000.0f / bdsp::calculateTimeInMs(BPMFollow->getValue(), phaserRateTimeParam.get(), phaserRateFracParam.get(), phaserRateDivisionParam.get(), true);
	phaser->setPhaseChangeRate(phaserRate);
	phaser->setCenterAndDepth(phaserCenterParam.getParameter()->getActualValueNormalized(), phaserDepthParam.get());
	phaser->setStereoWidth(phaserStereoParam.get());
	phaser->setFeedback(phaserFeedbackParam.get());

	phaser->setMix(generics[8]->mixParam.get());
	phaser->setBypassed(generics[8]->bypassParam.get());

	fxLatency += phaser->getLatency() * !phaser->isBypassed();


	//================================================================================================================================================================================================
	//Panner

	panner->setGain(pannerGainParam.get());
	panner->setPan(pannerPanParam.get());
	panner->setWidth(pannerWidthParam.get());

	panner->setMix(generics[9]->mixParam.get());
	panner->setBypassed(generics[9]->bypassParam.get());

	fxLatency += panner->getLatency() * !panner->isBypassed();


	//================================================================================================================================================================================================
	//Noise

	noise->setGain(noiseGainParam.get());
	noise->setDryMix(noiseDryParam.get());
	noise->setWetMix(1);
	noise->setColor(noiseColorParam.get());
	noise->setStereoWidth(noiseStereoParam.get());


	noise->setMix(generics[10]->mixParam.get());
	noise->setBypassed(generics[10]->bypassParam.get());

	fxLatency += noise->getLatency() * !noise->isBypassed();







	setLatencySamples(getBaseLatency() + fxLatency);
}

void FlexFXAudioProcessor::processSubBlock()
{

	inputSource->trackBuffer(subBlockBuffer);
	sidechainSource->trackBuffer(sideChainSubBlockBuffer);




	auto block = juce::dsp::AudioBlock<float>(subBlockBuffer);
	juce::dsp::ProcessContextReplacing<float> context(block);
	latencyAdjuster->process(context);

	chain->process(context);

}

void FlexFXAudioProcessor::processSubBlockBypassed()
{
}






//==============================================================================
//==============================================================================
void FlexFXAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{

	Processor::prepareToPlay(sampleRate, samplesPerBlock);
	const juce::dsp::ProcessSpec spec = { sampleRate, subBlockSpec->maximumBlockSize, 2 };


	inputSource->prepare(spec);
	sidechainSource->prepare(spec);



	distortion->prepare(spec);
	bitCrusher->prepare(spec);
	filter->prepare(spec);
	EQ->prepare(spec);
	pitchShifter->prepare(spec);
	ringMod->prepare(spec);

	ringModSource->prepare(spec);



	chorus->prepare(spec);
	flanger->prepare(spec);
	phaser->prepare(spec);
	panner->prepare(spec);

	pannerMeterController->prepare(spec);
	pannerMeterSource->prepare(spec);

	noise->prepare(spec);



	int maxLatency = 0;
	maxLatency += distortion->getMaxLatency();
	maxLatency += bitCrusher->getMaxLatency();
	maxLatency += filter->getMaxLatency();
	maxLatency += pitchShifter->getMaxLatency();
	maxLatency += chorus->getMaxLatency();
	maxLatency += flanger->getMaxLatency();
	maxLatency += phaser->getMaxLatency();
	maxLatency += panner->getMaxLatency();

	maxLatency += noise->getMaxLatency();





	chain->prepare(spec);

	latencyAdjuster->setMaxDelay(maxLatency);
	latencyAdjuster->prepare(spec);


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
	generics.clear();

	//================================================================================================================================================================================================
	//Distortion
	auto distTypeParam = dynamic_cast<juce::AudioParameterChoice*>(parameters->getParameter("DistortionTypeID"));
	distortionTypeParam.setParameter(distTypeParam);
	distortion->setParameter(distTypeParam);

	distortionPreGainParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("DistortionPreGainID")));
	distortionAmountParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("DistortionAmountID")));
	distortionAGCParam.setParameter(dynamic_cast<juce::AudioParameterBool*>(parameters->getParameter("DistortionGainCompensationID")));

	generics.add(new GenericFXParameters());
	generics.getLast()->init(parameters.get(), "Distortion");


	//================================================================================================================================================================================================
	//Bit Crush

	bitcrushDepthParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("BitCrushDepthID")));
	bitcrushRateParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("BitCrushRateID")));

	generics.add(new GenericFXParameters());
	generics.getLast()->init(parameters.get(), "BitCrush");

	//================================================================================================================================================================================================
	//Filter

	filterTypeParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("FilterTypeID")));
	filterFreqParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("FilterFrequencyID")));
	filterQParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("FilterResonanceID")));

	generics.add(new GenericFXParameters());
	generics.getLast()->init(parameters.get(), "Filter");

	//================================================================================================================================================================================================
	//EQ

	EQGlobalGainParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("EQGlobalGainID")));
	for (int j = 0; j < EQNumBands; ++j)
	{
		EQFreqParams[j].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("EQBand" + juce::String(j) + "FrequencyID")));
		EQBWParams[j].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("EQBand" + juce::String(j) + "QID")));
		EQGainParams[j].setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("EQBand" + juce::String(j) + "GainID")));
	}

	generics.add(new GenericFXParameters());
	generics.getLast()->init(parameters.get(), "EQ");

	//================================================================================================================================================================================================
	//Pitch Shift

	pitchShiftLeftParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("PitchShiftLeftID")));
	pitchShiftRightParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("PitchShiftRightID")));
	pitchShiftLinkParam.setParameter(dynamic_cast<juce::AudioParameterBool*>(parameters->getParameter("PitchShiftLinkID")));


	generics.add(new GenericFXParameters());
	generics.getLast()->init(parameters.get(), "PitchShift");

	//================================================================================================================================================================================================
	//Ring Mod

	ringModSourceParam.setParameter(dynamic_cast<juce::AudioParameterChoice*>(parameters->getParameter("RingModSourceID")));
	ringModShapeParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("RingModShapeID")));
	ringModSkewParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("RingModSkewID")));
	ringModFreqParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("RingModFrequencyID")));

	generics.add(new GenericFXParameters());
	generics.getLast()->init(parameters.get(), "RingMod");

	//================================================================================================================================================================================================
	//Chorus

	chorusRateTimeParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("ChorusRateHzRateID")));
	chorusRateFracParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("ChorusRateFractionID")));
	chorusRateDivisionParam.setParameter(dynamic_cast<juce::AudioParameterChoice*>(parameters->getParameter("ChorusRateDivisionID")));


	chorusDepthParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("ChorusDepthID")));
	chorusStereoParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("ChorusStereoID")));
	chorusVoicesParam.setParameter(dynamic_cast<juce::AudioParameterInt*>(parameters->getParameter("ChorusVoicesID")));


	generics.add(new GenericFXParameters());
	generics.getLast()->init(parameters.get(), "Chorus");

	//================================================================================================================================================================================================
	//Flanger

	flangerRateTimeParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("FlangerRateHzRateID")));
	flangerRateFracParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("FlangerRateFractionID")));
	flangerRateDivisionParam.setParameter(dynamic_cast<juce::AudioParameterChoice*>(parameters->getParameter("FlangerRateDivisionID")));


	flangerBaseParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("FlangerBaseID")));
	flangerDepthParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("FlangerDepthID")));
	flangerStereoParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("FlangerStereoID")));
	flangerFeedbackParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("FlangerFeedbackID")));


	generics.add(new GenericFXParameters());
	generics.getLast()->init(parameters.get(), "Flanger");

	//================================================================================================================================================================================================
	//Phaser

	phaserRateTimeParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("PhaserRateHzRateID")));
	phaserRateFracParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("PhaserRateFractionID")));
	phaserRateDivisionParam.setParameter(dynamic_cast<juce::AudioParameterChoice*>(parameters->getParameter("PhaserRateDivisionID")));


	phaserCenterParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("PhaserCenterID")));
	phaserDepthParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("PhaserDepthID")));
	phaserStereoParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("PhaserStereoID")));
	phaserFeedbackParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("PhaserFeedbackID")));


	generics.add(new GenericFXParameters());
	generics.getLast()->init(parameters.get(), "Phaser");

	//================================================================================================================================================================================================
	//Panner

	pannerGainParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("PannerGainID")));
	pannerPanParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("PannerPanID")));
	pannerWidthParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("PannerStereoWidthID")));


	generics.add(new GenericFXParameters());
	generics.getLast()->init(parameters.get(), "Panner");

	//================================================================================================================================================================================================
	//Noise

	noiseGainParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("NoiseGainID")));
	noiseDryParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("NoiseDryGainID")));
	noiseColorParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("NoiseColorID")));
	noiseStereoParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(parameters->getParameter("NoiseStereoID")));


	generics.add(new GenericFXParameters());
	generics.getLast()->init(parameters.get(), "Noise");
}


void FlexFXAudioProcessor::initParameters()
{
	loadParameterPointers();
	getParameterValues();

}

void FlexFXAudioProcessor::getParameterValues()
{
	bdsp::Processor::getParameterValues();

	for (auto* g : generics)
	{
		g->load();
	}

	//================================================================================================================================================================================================
	//Distortion
	auto distTypeParam = dynamic_cast<juce::AudioParameterChoice*>(parameters->getParameter("DistortionTypeID"));
	distortionTypeParam.load();


	distortionPreGainParam.load();
	distortionAmountParam.load();
	distortionAGCParam.load();


	//================================================================================================================================================================================================
	//Bit Crush

	bitcrushDepthParam.load();
	bitcrushRateParam.load();


	//================================================================================================================================================================================================
	//Filter

	filterTypeParam.load();
	filterFreqParam.load();
	filterQParam.load();


	//================================================================================================================================================================================================
	//EQ

	EQGlobalGainParam.load();
	for (int j = 0; j < EQNumBands; ++j)
	{
		EQFreqParams[j].load();
		EQBWParams[j].load();
		EQGainParams[j].load();
	}


	//================================================================================================================================================================================================
	//Pitch Shift

	pitchShiftLeftParam.load();
	pitchShiftRightParam.load();
	pitchShiftLinkParam.load();


	//================================================================================================================================================================================================
	//Ring Mod

	ringModSourceParam.load();
	ringModShapeParam.load();
	ringModSkewParam.load();
	ringModFreqParam.load();

	//================================================================================================================================================================================================
	//Chorus

	chorusRateTimeParam.load();
	chorusRateFracParam.load();
	chorusRateDivisionParam.load();


	chorusDepthParam.load();
	chorusStereoParam.load();
	chorusVoicesParam.load();

	//================================================================================================================================================================================================
	//Flanger

	flangerRateTimeParam.load();
	flangerRateFracParam.load();
	flangerRateDivisionParam.load();


	flangerBaseParam.load();
	flangerDepthParam.load();
	flangerStereoParam.load();
	flangerFeedbackParam.load();


	//================================================================================================================================================================================================
	//Phaser

	phaserRateTimeParam.load();
	phaserRateFracParam.load();
	phaserRateDivisionParam.load();


	phaserCenterParam.load();
	phaserDepthParam.load();
	phaserStereoParam.load();
	phaserFeedbackParam.load();


	//================================================================================================================================================================================================
	//Panner

	pannerGainParam.load();
	pannerPanParam.load();
	pannerWidthParam.load();


	//================================================================================================================================================================================================
	//Noise

	noiseGainParam.load();
	noiseDryParam.load();
	noiseColorParam.load();
	noiseStereoParam.load();


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
//
//void FlexFXAudioProcessor::reorderProcessors(int indexMoved, int indexMovedTo)
//{
//	processorOrder.move(indexMoved, indexMovedTo);
//	for (int i = 0; i < processorOrder.size(); ++i)
//	{
//		auto indexParam = generics[processorOrder[i]]->indexParam.getParameter();
//		indexParam->beginChangeGesture();
//		indexParam->setValueNotifyingHost(indexParam->convertTo0to1(i));
//		indexParam->endChangeGesture();
//	}
//}

