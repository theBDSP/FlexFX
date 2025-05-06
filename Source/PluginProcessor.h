/*
  ==============================================================================

	This dir contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "Texture.h"
//#include "Presets.h"




#define EQNumBands 5

//==============================================================================
class FlexFXAudioProcessorEditor;

class FlexFXAudioProcessor : public bdsp::Processor
{
public:
	//==============================================================================
	FlexFXAudioProcessor();
	~FlexFXAudioProcessor() override;

	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;



	void setBypass(bool bypassState) const;

	//==============================================================================
	juce::AudioProcessor* createPluginFilter();
	juce::AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;

	//==============================================================================


	void swapSlotParameterValues(int idx1, int idx2);

	void loadParameterPointers();
	void initParameters()override;
	void getParameterValues() override;

	juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() override;


#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif



	juce::StringArray FXTypeNames;
	enum FXTypes :int { Distortion, BitCrush, Filter, ParamEQ, PitchShift, RingMod, Chorus, Flanger, Phaser, Panner, Noise };


	std::unique_ptr<bdsp::dsp::EmptyProcessor<float>> emptyProcessor;
	std::unique_ptr<bdsp::dsp::VariableDistortion<float>> distortion;
	std::unique_ptr<bdsp::dsp::BitCrushDistortion<float>> bitCrusher;
	std::unique_ptr<bdsp::dsp::CascadedFilter<float, bdsp::dsp::BiQuadFilters::StateVariableFilter<float>, 2>> filter;
	std::unique_ptr<bdsp::dsp::ParametricEQ<float>> EQ;
	std::unique_ptr<bdsp::dsp::PitchShifter<float>> pitchShifter;
	std::unique_ptr<bdsp::dsp::RingModulation<float>> ringMod;
	std::unique_ptr<bdsp::dsp::Chorus<float>> chorus;
	std::unique_ptr<bdsp::dsp::Flanger<float>> flanger;
	std::unique_ptr<bdsp::dsp::Phaser<float>> phaser;


	std::unique_ptr<bdsp::dsp::StereoWidener<float>> panner;

	std::unique_ptr<bdsp::LevelMeterController<float>> pannerMeterController;
	std::unique_ptr<bdsp::dsp::SampleSource<float>> pannerMeterSource;


	std::unique_ptr<bdsp::dsp::SampleSource<float>> ringModSource;

	std::unique_ptr<bdsp::dsp::Compressor<float>> compressor;
	std::unique_ptr<bdsp::dsp::Noise::StereoNoiseGenerator<float, bdsp::dsp::Noise::ColoredNoise<float>>> noise;




	struct GenericFXParameters
	{
		void init(juce::AudioProcessorValueTreeState* APVTS, const juce::String& name)
		{
			mixParam.setParameter(dynamic_cast<bdsp::ControlParameter*>(APVTS->getParameter(name + "MixID")));
			bypassParam.setParameter(dynamic_cast<juce::AudioParameterBool*>(APVTS->getParameter(name + "BypassID")));
			indexParam.setParameter(dynamic_cast<juce::AudioParameterInt*>(APVTS->getParameter(name + "IndexID")));
		}
		void load()
		{
			mixParam.load();
			bypassParam.load();
			indexParam.load();
		}



		bdsp::ParameterPointerControl mixParam;
		bdsp::ParameterPointerBool bypassParam;
		bdsp::ParameterPointerInt indexParam;
	};

	juce::OwnedArray<GenericFXParameters> generics;

	void reorderProcessors(int indexMoved, int indexMovedTo);
private:

	juce::Array<int> processorOrder;

	std::unique_ptr<bdsp::dsp::ProcessorChain<float>> chain;



	std::unique_ptr<bdsp::dsp::DelayLineBase<float>> latencyAdjuster; // changes input delay for each chain to align for latency of FX

	int fxLatency;




	//================================================================================================================================================================================================
	//Distortion 

	bdsp::ParameterPointerChoice distortionTypeParam;

	bdsp::ParameterPointerControl distortionPreGainParam;

	bdsp::ParameterPointerControl distortionAmountParam;

	bdsp::ParameterPointerBool distortionAGCParam;


	//================================================================================================================================================================================================
	//Bit Crush 


	bdsp::ParameterPointerControl bitcrushDepthParam;

	bdsp::ParameterPointerControl bitcrushRateParam;


	//================================================================================================================================================================================================
	//Filter


	bdsp::ParameterPointerControl filterTypeParam;

	bdsp::ParameterPointerControl filterFreqParam;

	bdsp::ParameterPointerControl filterQParam;




	//================================================================================================================================================================================================
	//PitchShift


	bdsp::ParameterPointerControl pitchShiftLeftParam;

	bdsp::ParameterPointerControl pitchShiftRightParam;

	bdsp::ParameterPointerBool pitchShiftLinkParam;



	//================================================================================================================================================================================================
	//RingMod


	bdsp::ParameterPointerChoice ringModSourceParam;

	bdsp::ParameterPointerControl ringModShapeParam;
	bdsp::ParameterPointerControl ringModSkewParam;
	bdsp::ParameterPointerControl ringModFreqParam;



	//================================================================================================================================================================================================
	//Chorus


	bdsp::ParameterPointerControl chorusRateTimeParam;

	bdsp::ParameterPointerControl chorusRateFracParam;

	bdsp::ParameterPointerChoice chorusRateDivisionParam;



	bdsp::ParameterPointerControl chorusDepthParam;

	bdsp::ParameterPointerControl chorusStereoParam;


	bdsp::ParameterPointerInt chorusVoicesParam;



	//================================================================================================================================================================================================
	//Flanger


	bdsp::ParameterPointerControl flangerRateTimeParam;

	bdsp::ParameterPointerControl flangerRateFracParam;

	bdsp::ParameterPointerChoice flangerRateDivisionParam;



	bdsp::ParameterPointerControl flangerBaseParam;

	bdsp::ParameterPointerControl flangerDepthParam;

	bdsp::ParameterPointerControl flangerStereoParam;

	bdsp::ParameterPointerControl flangerFeedbackParam;



	//================================================================================================================================================================================================
	//Phaser


	bdsp::ParameterPointerControl phaserRateTimeParam;

	bdsp::ParameterPointerControl phaserRateFracParam;

	bdsp::ParameterPointerChoice phaserRateDivisionParam;



	bdsp::ParameterPointerControl phaserCenterParam;

	bdsp::ParameterPointerControl phaserDepthParam;

	bdsp::ParameterPointerControl phaserStereoParam;

	bdsp::ParameterPointerControl phaserFeedbackParam;



	//================================================================================================================================================================================================
	//Panner



	bdsp::ParameterPointerControl pannerGainParam;

	bdsp::ParameterPointerControl pannerPanParam;

	bdsp::ParameterPointerControl pannerWidthParam;



	//================================================================================================================================================================================================
	//Noise



	bdsp::ParameterPointerControl noiseDryParam;
	bdsp::ParameterPointerControl noiseGainParam;

	bdsp::ParameterPointerControl noiseColorParam;


	bdsp::ParameterPointerControl noiseStereoParam;



	//================================================================================================================================================================================================
	//EQ


	bdsp::ParameterPointerControl EQFreqParams[EQNumBands];

	bdsp::ParameterPointerControl EQBWParams[EQNumBands];

	bdsp::ParameterPointerControl EQGainParams[EQNumBands];

	bdsp::ParameterPointerControl EQGlobalGainParam;



	//================================================================================================================================================================================================

	void createBypassAndMixParam(juce::AudioProcessorValueTreeState::ParameterLayout& layout, const juce::String& FXName, int defaultIndex);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlexFXAudioProcessor)

		// Inherited via Processor
		void setFactoryPresets() override;





	// Inherited via Processor
	void processBlockInit(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

	void processSubBlock() override;


	// Inherited via Processor
	void processSubBlockBypassed() override;



	bdsp::dsp::ProcessorChain<float>* addFX(int chainIndex, int fxIndex);

};

