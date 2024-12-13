#pragma once


#include "PluginProcessor.h"

#include "font.h"


//==============================================================================

class FlexFXAudioProcessorEditor : public bdsp::Editor
{
public:
	FlexFXAudioProcessorEditor(FlexFXAudioProcessor& p);

	~FlexFXAudioProcessorEditor() override;

	//==============================================================================
	void paint(juce::Graphics&) override;

	void resized() override;

	void setUniversals() override;







	std::unique_ptr<bdsp::BasicContainerComponent> titleBar;








	juce::StringArray* FXTypeNames;
private:




	float border;

	juce::Rectangle<int> fxSectionSize;




	juce::Rectangle<float> topBarArea, modArea, macroArea, FXArea, hintBarArea;

	bdsp::RangedCircleSlider* createRangedCircleSlider(const juce::String& name, juce::Component* parentSection, juce::OwnedArray<bdsp::RangedCircleSlider>& arrayToAddTo, const bdsp::NamedColorsIdentifier& color, const juce::String& label);

	void initFX()
	{
		initDistortions();
		initBitcrushes();
		initFilters();
		initEQs();
		initPitchShifts();
		initRingMods();
		initChoruses();
		initFlangers();
		initPhasers();
		initPanners();
		initNoises();

		for (int i = 0; i < numFXSlots; ++i)
		{
			auto selector = dynamic_cast<FXSlot*>(fxSlotManager->getComp(i))->fxTypeSelector.get();
			selector->onChange(selector->getIndex());
		}
	}
	void resizeFX()
	{
		resizeDistortions();
		resizeBitcrushes();
		resizeFilters();
		resizeEQs();
		resizePitchShifts();
		resizeRingMods();
		resizeChoruses();
		resizeFlangers();
		resizePhasers();
		resizePanners();
		resizeNoises();
	}
	void swapFX(int idx1, int idx2)
	{
		swapDistortions(idx1, idx2);
		swapBitcrushes(idx1, idx2);
		swapFilters(idx1, idx2);
		swapEQs(idx1, idx2);
		swapPitchShifts(idx1, idx2);
		swapRingMods(idx1, idx2);
		swapChoruses(idx1, idx2);
		swapFlangers(idx1, idx2);
		swapPhasers(idx1, idx2);
		swapPanners(idx1, idx2);
		swapNoises(idx1, idx2);
	}


	//================================================================================================================================================================================================
	//Distortion
	juce::OwnedArray<bdsp::Component> distortionSections;

	juce::OwnedArray<bdsp::RangedCircleSlider> distortionPreGainSliders, distortionAmountSliders, distortionMixSliders;
	juce::OwnedArray<bdsp::ToggleSlider> distortionAGCButtons;
	juce::OwnedArray<bdsp::MousePassthrough<bdsp::Label>> distortionAGCLabels;
	juce::OwnedArray<bdsp::Component> distortionAGCSections;
	juce::OwnedArray<bdsp::DistortionVisualizer> distortionVisualizers;
	juce::OwnedArray<bdsp::IncrementalComboBox<bdsp::AdvancedComboBox>> distortionTypeSelectors;

	void initDistortions();
	void resizeDistortions();
	void swapDistortions(int idx1, int idx2);

	//================================================================================================================================================================================================
	//Bit Crush
	juce::OwnedArray<bdsp::Component> bitcrushSections;

	juce::OwnedArray<bdsp::RangedCircleSlider> bitcrushDepthSliders, bitcrushRateSliders, bitcrushMixSliders;

	juce::OwnedArray<bdsp::BitCrushVisualizer> bitcrushVisualizers;

	void initBitcrushes();
	void resizeBitcrushes();
	void swapBitcrushes(int idx1, int idx2);


	//================================================================================================================================================================================================
	//Filter
	juce::OwnedArray<bdsp::Component> filterSections;

	juce::OwnedArray<bdsp::RangedCircleSlider> filterTypeSliders, filterFreqSliders, filterQSliders, filterMixSliders;

	juce::OwnedArray<bdsp::FilterVisualizer> filterVisualizers;

	void initFilters();
	void resizeFilters();
	void swapFilters(int idx1, int idx2);


	//================================================================================================================================================================================================
	//EQ
	juce::OwnedArray<bdsp::Component> EQSections;

	juce::OwnedArray<bdsp::TabsComponent> EQTabs;
	juce::OwnedArray<bdsp::RangedCircleSlider>EQGainSliders[numFXSlots], EQFreqSliders[numFXSlots], EQBWSLiders[numFXSlots];
	juce::OwnedArray<bdsp::RangedCircleSlider> EQMixSliders, EQGlobalGainSliders;
	juce::OwnedArray<bdsp::EQVisualizer> EQVisualizers;

	void initEQs();
	void resizeEQs();
	void swapEQs(int idx1, int idx2);

	//================================================================================================================================================================================================
	//PitchShift
	juce::OwnedArray<bdsp::Component> pitchShiftSections;

	juce::OwnedArray<bdsp::RangedCircleSlider> pitchShiftLeftSliders, pitchShiftRightSliders, pitchShiftLinkDummySliders, pitchShiftMixSliders;
	juce::OwnedArray<bdsp::PathButton> pitchShiftLinkButtons;

	juce::OwnedArray<bdsp::PitchShifterVisualizer> pitchShiftVisualizers;

	void initPitchShifts();
	void resizePitchShifts();
	void swapPitchShifts(int idx1, int idx2);
	
	//================================================================================================================================================================================================
	//RingMod
	juce::OwnedArray<bdsp::Component> ringModSections;

	juce::OwnedArray<bdsp::RangedCircleSlider> ringModShapeSliders, ringModSkewSliders, ringModFreqSliders, ringModMixSliders;
	juce::OwnedArray<bdsp::ComboBox> ringModSourceCombos;
	juce::OwnedArray<bdsp::RingModVisualizer> ringModVisualizers;

	void initRingMods();
	void resizeRingMods();
	void swapRingMods(int idx1, int idx2);

	//================================================================================================================================================================================================
	//Chorus
	juce::OwnedArray<bdsp::Component> chorusSections;

	juce::OwnedArray<bdsp::RangedCircleSlider> chorusDepthSliders, chorusStereoWidthSliders, chorusMixSlider;
	juce::OwnedArray<bdsp::RangedSyncFraction> chorusRateFracs;
	juce::OwnedArray<bdsp::CircleSlider> chorusVoiceSliders;
	juce::OwnedArray<bdsp::ChorusVisualizer> chorusVisualizers;

	void initChoruses();
	void resizeChoruses();
	void swapChoruses(int idx1, int idx2);

	//================================================================================================================================================================================================
	//Flanger
	juce::OwnedArray<bdsp::Component> flangerSections;

	juce::OwnedArray<bdsp::RangedCircleSlider> flangerBaseSliders, flangerDepthSliders, flangerStereoWidthSliders, flangerFeedbackSliders, flangerMixSliders;
	juce::OwnedArray<bdsp::RangedSyncFraction> flangerRateFracs;
	juce::OwnedArray<bdsp::FlangerVisualizer> flangerVisualizers;

	void initFlangers();
	void resizeFlangers();
	void swapFlangers(int idx1, int idx2);

	//================================================================================================================================================================================================
	//Phaser
	juce::OwnedArray<bdsp::Component> phaserSections;

	juce::OwnedArray<bdsp::RangedCircleSlider> phaserCenterSliders, phaserDepthSliders, phaserStereoWidthSliders, phaserFeedbackSliders, phaserMixSliders;
	juce::OwnedArray<bdsp::RangedSyncFraction> phaserRateFracs;
	juce::OwnedArray<bdsp::PhaserVisualizer> phaserVisualizers;

	void initPhasers();
	void resizePhasers();
	void swapPhasers(int idx1, int idx2);

	//================================================================================================================================================================================================
	//Panner
	juce::OwnedArray<bdsp::Component> pannerSections;

	juce::OwnedArray<bdsp::RangedCircleSlider> pannerGainSliders, pannerPanSliders, pannerWidthSliders, pannerMixSliders;
	juce::OwnedArray<bdsp::LevelMeter<float>> pannerMeters;

	void initPanners();
	void resizePanners();
	void swapPanners(int idx1, int idx2);


	//================================================================================================================================================================================================
	//Noise
	juce::OwnedArray<bdsp::Component> NoiseSections;

	juce::OwnedArray<bdsp::RangedCircleSlider> noiseGainSliders, noiseColorSliders, noiseStereoSliders, noiseDrySliders;
	juce::OwnedArray<bdsp::NoiseVisualizer> noiseVisualizers;


	void initNoises();
	void resizeNoises();
	void swapNoises(int idx1, int idx2);

	//================================================================================================================================================================================================

	int maxWidth = 1500;



	class FXSlot : public bdsp::Component, public bdsp::GUI_Universals::Listener
	{
	public:
		FXSlot(FlexFXAudioProcessorEditor* editor, int slotNum, bdsp::Button* chainBypassButton);
		virtual ~FXSlot() override
		{

		}

		void paint(juce::Graphics& g) override;
		void resized() override;

		void setFXType(int idx);
		void visibilityChanged() override;


		int getSlotIndex() const;


		void GUI_UniversalsChanged() override;

		juce::Rectangle<float> getDraghandleRect();
		juce::Rectangle<int> getFXSectionBounds();

		void swapWithOtherSlot(FXSlot* otherSlot);

		bdsp::Component* getfxSectionHolder()
		{
			return &fxSectionHolder;
		}

		bdsp::PathButton* getBypassButton()
		{
			return bypassButton.get();
		}

		std::unique_ptr<bdsp::IncrementalComboBox<>> fxTypeSelector;
		bdsp::NamedColorsIdentifier currentColor;

	private:
		std::unique_ptr<bdsp::PathButton> bypassButton;

		bdsp::Component fxSectionHolder;
		bdsp::Component* currentFXSection = nullptr;

		FlexFXAudioProcessorEditor* p = nullptr;

		int slotIndex;
		juce::String title;
		float border = 0;


	};


	class FXChainManager : public bdsp::RearrangeableTabsComponent
	{
	public:

		FXChainManager(FlexFXAudioProcessorEditor* editor);
		void resized() override;
		void paint(juce::Graphics& g) override;

		void addComponent(int i);
		FXSlot* getComp(int i);

		void setDragColor(int idx, const bdsp::NamedColorsIdentifier& c)
		{
			rearrangeComp->setSingleDragHandleColors(idx, c, c, BDSP_COLOR_PURE_BLACK);

		}
		void setDragText(int idx, const juce::String& s)
		{
			rearrangeComp->setSingleDragHandleText(idx, s);
		}

	private:
		juce::Array<FXSlot*> compCasts;

		FlexFXAudioProcessorEditor* p = nullptr;

		juce::OwnedArray<bdsp::PathButton> chainBypassButtons;
		std::unique_ptr<bdsp::TextButton> routingButton;
		std::unique_ptr<bdsp::PathButton> swapOrder;
		std::unique_ptr<bdsp::RangedContainerSlider> parallelMixSlider;
	};

	class AnimatedTitle : public bdsp::OpenGLComponent
	{
	public:
		AnimatedTitle(FlexFXAudioProcessorEditor* parent);
		~AnimatedTitle();

		void paintOverChildren(juce::Graphics& g) override;

		void resized() override;

		void generateVertexBuffer() override;


	private:

		bdsp::NamedColorsIdentifier background;
		float colorProgress = 0;
		float colorProgressInc = 0.00025;

		float rotationProgress = 0;
		float rotationProgressInc = 0.00033;
		FlexFXAudioProcessorEditor* e;
		juce::Path cutout;

		juce::Array<float> reds, greens, blues;


	};




	std::unique_ptr<AnimatedTitle> title;

	std::unique_ptr<FXChainManager>fxSlotManager;
	std::unique_ptr<bdsp::TabsComponent> modTabs;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlexFXAudioProcessorEditor)







};

