/*
  ==============================================================================

	This dir contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "Texture.h"
//#include "Presets.h"




constexpr auto numFXSlots = 8;
constexpr auto numFXPerChain = 4;
constexpr auto numFXChains = 2;

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




	void loadParameterPointers();
	void initParameters()override;
	void getParameterValues() override;

	juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() override;


#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif



	juce::StringArray FXTypeNames;
	enum FXTypes :int { None, Distortion, BitCrush, Filter, EQ, PitchShift, RingMod, Chorus, Flanger, Phaser, Panner, Noise, NUM };


	juce::OwnedArray<bdsp::dsp::EmptyProcessor<float>> emptyProcessors;
	juce::OwnedArray<bdsp::dsp::VariableDistortion<float>> distortions;
	juce::OwnedArray<bdsp::dsp::BitCrushDistortion<float>> bitCrushes;
	juce::OwnedArray<bdsp::dsp::CascadedFilter<float, bdsp::dsp::BiQuadFilters::StateVariableFilter<float>, 2>> filters;
	juce::OwnedArray<bdsp::dsp::StereoPitchShifter<float>> pitchShifters;
	juce::OwnedArray<bdsp::dsp::RingModulation<float>> ringMods;
	juce::OwnedArray<bdsp::dsp::Chorus<float>> choruses;
	juce::OwnedArray<bdsp::dsp::Flanger<float>> flangers;
	juce::OwnedArray<bdsp::dsp::Phaser<float>> phasers;


	juce::OwnedArray<bdsp::dsp::StereoWidener<float>> panners;

	juce::OwnedArray<bdsp::LevelMeterController<float>> pannerMeterControllers;
	juce::OwnedArray<bdsp::dsp::SampleSource<float>> pannerMeterSources;


	juce::OwnedArray<bdsp::SpectrogramController> ringModControllers;
	juce::OwnedArray<bdsp::dsp::SampleSource<float>> ringModSources;

	juce::OwnedArray<bdsp::dsp::Compressor<float>> compressors;
	juce::OwnedArray<bdsp::dsp::Noise::StereoNoiseGenerator<float, bdsp::dsp::Noise::ColoredNoise<float>>> noises;

	juce::OwnedArray<bdsp::dsp::ParametricEQ<float>> EQs;
private:


	juce::Array <bdsp::dsp::BaseProcessingUnit<float>*> currentFXs;
	juce::OwnedArray<bdsp::dsp::ProcessorChain<float>> chains;

	bdsp::ParameterPointerBool routingParam;

	bdsp::ParameterPointerBool chainBypassParams[numFXChains];

	bdsp::ParameterPointerControl parallelMixParam;

	bool chainIncluded[numFXChains];
	juce::OwnedArray<bdsp::dsp::DelayLineBase<float>> latencyAdjusters; // changes input delay for each chain to align for latency of FX

	int fxLatency[numFXSlots];
	int chainLatency[numFXChains];

	float maxSingleFXLatency = 0;

	juce::OwnedArray<juce::AudioBuffer<float>> chainBuffers;



	bdsp::ParameterPointerControl fxMixParams[numFXSlots];


	bdsp::ParameterPointerChoice fxTypeParams[numFXSlots];


	bdsp::ParameterPointerBool fxBypassParams[numFXSlots];

	//================================================================================================================================================================================================
	//Distortion 

	bdsp::ParameterPointerChoice distortionTypeParams[numFXSlots];

	bdsp::ParameterPointerControl distortionPreGainParams[numFXSlots];

	bdsp::ParameterPointerControl distortionAmountParams[numFXSlots];

	bdsp::ParameterPointerBool distortionAGCParams[numFXSlots];

	//================================================================================================================================================================================================
	//Bit Crush 


	bdsp::ParameterPointerControl bitcrushDepthParams[numFXSlots];

	bdsp::ParameterPointerControl bitcrushRateParams[numFXSlots];

	//================================================================================================================================================================================================
	//Filter


	bdsp::ParameterPointerControl filterTypeParams[numFXSlots];

	bdsp::ParameterPointerControl filterFreqParams[numFXSlots];

	bdsp::ParameterPointerControl filterQParams[numFXSlots];



	//================================================================================================================================================================================================
	//PitchShift


	bdsp::ParameterPointerControl pitchShiftLeftParams[numFXSlots];

	bdsp::ParameterPointerControl pitchShiftRightParams[numFXSlots];

	bdsp::ParameterPointerBool pitchShiftLinkParams[numFXSlots];

	//================================================================================================================================================================================================
	//RingMod


	bdsp::ParameterPointerChoice ringModSourceParams[numFXSlots];

	bdsp::ParameterPointerControl ringModShapeParams[numFXSlots];
	bdsp::ParameterPointerControl ringModSkewParams[numFXSlots];
	bdsp::ParameterPointerControl ringModFreqParams[numFXSlots];


	//================================================================================================================================================================================================
	//Chorus


	bdsp::ParameterPointerControl chorusRateTimeParams[numFXSlots];

	bdsp::ParameterPointerControl chorusRateFracParams[numFXSlots];

	bdsp::ParameterPointerChoice chorusRateDivisionParams[numFXSlots];



	bdsp::ParameterPointerControl chorusDepthParams[numFXSlots];

	bdsp::ParameterPointerControl chorusStereoParams[numFXSlots];


	bdsp::ParameterPointerInt chorusVoicesParams[numFXSlots];

	//================================================================================================================================================================================================
	//Flanger


	bdsp::ParameterPointerControl flangerRateTimeParams[numFXSlots];

	bdsp::ParameterPointerControl flangerRateFracParams[numFXSlots];

	bdsp::ParameterPointerChoice flangerRateDivisionParams[numFXSlots];



	bdsp::ParameterPointerControl flangerBaseParams[numFXSlots];

	bdsp::ParameterPointerControl flangerDepthParams[numFXSlots];

	bdsp::ParameterPointerControl flangerStereoParams[numFXSlots];

	bdsp::ParameterPointerControl flangerFeedbackParams[numFXSlots];


	//================================================================================================================================================================================================
	//Phaser


	bdsp::ParameterPointerControl phaserRateTimeParams[numFXSlots];

	bdsp::ParameterPointerControl phaserRateFracParams[numFXSlots];

	bdsp::ParameterPointerChoice phaserRateDivisionParams[numFXSlots];



	bdsp::ParameterPointerControl phaserCenterParams[numFXSlots];

	bdsp::ParameterPointerControl phaserDepthParams[numFXSlots];

	bdsp::ParameterPointerControl phaserStereoParams[numFXSlots];

	bdsp::ParameterPointerControl phaserFeedbackParams[numFXSlots];


	//int phaserStageVals[numFXSlots];
	//bdsp::ParameterPointerInt phaserStageParams[numFXSlots];
	//================================================================================================================================================================================================
	//Panner



	bdsp::ParameterPointerControl pannerGainParams[numFXSlots];

	bdsp::ParameterPointerControl pannerPanParams[numFXSlots];

	bdsp::ParameterPointerControl pannerWidthParams[numFXSlots];



	//================================================================================================================================================================================================
	//Noise



	bdsp::ParameterPointerControl noiseDryParams[numFXSlots];
	bdsp::ParameterPointerControl noiseGainParams[numFXSlots];

	bdsp::ParameterPointerControl noiseColorParams[numFXSlots];


	bdsp::ParameterPointerControl noiseStereoParams[numFXSlots];




	//================================================================================================================================================================================================
	//EQ


	bdsp::ParameterPointerControl EQFreqParams[numFXSlots][EQNumBands];

	bdsp::ParameterPointerControl EQBWParams[numFXSlots][EQNumBands];

	bdsp::ParameterPointerControl EQGainParams[numFXSlots][EQNumBands];

	bdsp::ParameterPointerControl EQGlobalGainParams[numFXSlots];


	//================================================================================================================================================================================================



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
