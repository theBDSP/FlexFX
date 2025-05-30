#pragma once


#include "PluginProcessor.h"

#include "font.h"
#include <juce_audio_processors/processors/juce_AudioProcessorParameter.h>


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






	std::unique_ptr<bdsp::Component> titleBar;


	std::unique_ptr<bdsp::TabsComponent> modulationTabs;





	juce::StringArray* FXTypeNames;
private:




	float border;




	juce::Rectangle<float> topBarArea, macroArea, FXArea, hintBarArea;


	class FXUI : public bdsp::Component
	{
	public:
		FXUI(FlexFXAudioProcessorEditor* e, int fxIdx)
			:bdsp::Component(&e->GUIUniversals)
		{
			editor = e;
			FXTypeIndex = fxIdx;
			setFontIndex(1);
		}

		void resized() override
		{
			titleRect = juce::Rectangle<float>(getWidth(), getWidth() / 15);
			usableRect = getLocalBounds().toFloat().withTop(titleRect.getBottom()).reduced(universals->dividerSize);
		}

		void paint(juce::Graphics& g) override
		{


			auto f = bdsp::resizeFontToFit(getFont().withExtraKerningFactor(0.1), titleRect.getWidth(), titleRect.getHeight(), editor->fxSlotManager->longestTitle);

			//g.setColour(getColor(editor->FXTypeNames->operator[](FXTypeIndex)));
			auto titlePath = bdsp::createTextPath(f, editor->FXTypeNames->operator[](FXTypeIndex), juce::Rectangle<float>(titleRect.getWidth(), titleRect.getHeight()));

			titlePath.applyTransform(juce::AffineTransform().translated(titleRect.getBottomLeft() - titlePath.getBounds().getBottomLeft() + juce::Point<float>(universals->rectThicc, -universals->rectThicc)));

			g.setColour(getColor(BDSP_COLOR_MID));
			g.fillRoundedRectangle(usableRect.expanded(universals->dividerSize), universals->roundedRectangleCurve);

			g.setColour(getColor(BDSP_COLOR_BLACK));
			g.fillRoundedRectangle(usableRect, universals->roundedRectangleCurve);


			g.setColour(getColor(BDSP_COLOR_WHITE));
			g.fillPath(titlePath);

		}

		std::unique_ptr<bdsp::TextButton>& getBypassButton()
		{
			return bypass;
		}

		int getFXTypeIndex()
		{
			return FXTypeIndex;
		}

		juce::AudioParameterInt* getIndexParameter()
		{
			return indexParam;
		}

		virtual float getDesiredAspectRatio()
		{
			return 1.5;
		}

	protected:
		void createRangedCircleSlider(const juce::String& name, std::unique_ptr<bdsp::RangedCircleSlider>& smartPointer, const bdsp::NamedColorsIdentifier& color, const juce::String& label)
		{
			jassert(!smartPointer.operator bool());
			smartPointer = std::make_unique<bdsp::RangedCircleSlider>(universals, name);
			bdsp::CircleSlider& current = *smartPointer.get();





			addAndMakeVisible(smartPointer.get());
			current.attach(*editor->audioProcessor.parameters.get(), current.getComponentID());

			current.setSliderColor(color, BDSP_COLOR_WHITE);
			current.label.setTitle(label);
			current.initText();
		}

		void createMixAndBypass(const juce::String& name, const bdsp::NamedColorsIdentifier& color)
		{
			createRangedCircleSlider(name + " Mix", mixSlider, color, "Mix");
			mixSlider->setHintBarText("Dry/Wet mix of the " + name);

			bypass = std::make_unique<bdsp::TextButton>(universals);
			bypass->setClickingTogglesState(true);
			bypass->setComponentID("BypassID");
			bypass->onClick = [=]()
			{
				//for (auto* c : getChildren())
				//{
				//	if (c->getComponentID() != bypass->getComponentID())
				//	{
				//		c->setEnabled(!bypass->getToggleState());
				//	}
				//}
				bypass->setHintBarText((bypass->getToggleState() ? "Enables" : "Disables") + juce::String(" the ") + name);
			};
			auto trimmed = name.removeCharacters(" ");

			bypass->attach(*editor->audioProcessor.parameters.get(), trimmed + "BypassID");
			bypass->backgroundDown = color.withMultipliedAlpha(universals->disabledAlpha);
			bypass->backgroundUp = color;
			bypass->setCorners(bdsp::CornerCurves(bdsp::CornerCurves::topLeft | bdsp::CornerCurves::bottomLeft));
			bypass->setHasOutline(false);

			indexParam = dynamic_cast<juce::AudioParameterInt*>(editor->audioProcessor.parameters->getParameter(trimmed + "IndexID"));
		}


		std::unique_ptr<bdsp::RangedCircleSlider> mixSlider;
		std::unique_ptr<bdsp::TextButton> bypass;
		FlexFXAudioProcessorEditor* editor = nullptr;

		juce::AudioParameterInt* indexParam = nullptr;

		int FXTypeIndex;
		juce::Rectangle<float> titleRect, usableRect;

	};


	class DistortionUI : public FXUI
	{
	public:
		DistortionUI(FlexFXAudioProcessorEditor* e)
			:FXUI(e, 0)
		{
			auto color = bdsp::NamedColorsIdentifier(e->FXTypeNames->operator[](FlexFXAudioProcessor::Distortion));


			universals->hintBar->addTextToRecolor({ "Distortion", color });
			universals->hintBar->addTextToRecolor({ "Distortion's", color });
			//================================================================================================================================================================================================

			createRangedCircleSlider("Distortion Pre Gain", preGainSlider, color, "Gain");
			preGainSlider->setHintBarText("Gain of input into Distortion");
			preGainSlider->slider.addCatchValues({ 1 }, false);

			//================================================================================================================================================================================================

			createRangedCircleSlider("Distortion Amount", amountSlider, color, "Amt");
			amountSlider->setHintBarText("Intensity of Distortion");

			//================================================================================================================================================================================================

			createMixAndBypass("Distortion", color);

			//================================================================================================================================================================================================
			AGCSection = std::make_unique<bdsp::Component>(universals);
			addAndMakeVisible(AGCSection.get());

			AGCButton = std::make_unique< bdsp::ToggleSlider>(universals, "");
			auto currentAGC = AGCButton.get();


			currentAGC->setFontIndex(2);
			currentAGC->setColors(color, bdsp::NamedColorsIdentifier());

			currentAGC->attach(*e->audioProcessor.parameters.get(), "DistortionGainCompensationID");

			AGCSection->addAndMakeVisible(AGCButton.get());

			currentAGC->onStateChange = [=]()
			{
				currentAGC->setHintBarText(juce::String(currentAGC->getToggleState() ? "Disables " : "Enables ") + "automatic gain compensation on Distortion's output");
			};

			AGCLabel = std::make_unique<bdsp::MousePassthrough<bdsp::Label>>(currentAGC, universals);

			auto currentAGCLabel = AGCLabel.get();
			AGCSection->addAndMakeVisible(AGCLabel.get());

			currentAGCLabel->setText("Auto", juce::sendNotification);
			//		//currentAGCLabel->setLookAndFeel(universals.MasterCircleSliderLNF);
			currentAGCLabel->setJustificationType(juce::Justification::centred);
			currentAGCLabel->setColour(juce::Label::textColourId, getColor(color));
			//================================================================================================================================================================================================

			visualizer = std::make_unique< bdsp::DistortionVisualizer>(universals, &e->audioProcessor.lookups, &amountSlider->slider, &mixSlider->slider, currentAGC);
			addAndMakeVisible(visualizer.get());
			visualizer->getVis()->setColor(color);
			visualizer->getVis()->setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_BLACK);

			//================================================================================================================================================================================================

			auto DistortionTypeParam = dynamic_cast<juce::AudioParameterChoice*>(e->audioProcessor.parameters->getParameter("DistortionTypeID"));

			juce::Array<juce::Path> DistortionTypeShapes;
			for (int i = 0; i < DistortionTypeParam->choices.size(); ++i)
			{
				auto p = e->audioProcessor.lookups.distortionLookups->nameToDistortionType(DistortionTypeParam->choices[i])->getIcon(false);
				juce::PathStrokeType(p.getBounds().getWidth() * 0.1).createStrokedPath(p, p);
				DistortionTypeShapes.add(p);
			}



			typeSelector = std::make_unique<bdsp::IncrementalComboBox<bdsp::AdvancedComboBox>>(DistortionTypeParam, universals, DistortionTypeShapes);

			typeSelector->setIconBorderSize(juce::BorderSize<float>(0.1, 0, 0.1, 0.1));
			typeSelector->onChange = [=](int i)
			{
				auto text = DistortionTypeParam->getCurrentChoiceName();
				auto type = e->audioProcessor.lookups.distortionLookups->nameToDistortionType(text);

				visualizer->getVis()->setType(type);


				typeSelector->setHintBarText("Transfer function of Distortion");

			};
			typeSelector->setMenuWidth(1);
			typeSelector->setFontIndex(0);
			typeSelector->setJustification(juce::Justification::centredLeft);

			typeSelector->setRowsAndCols(DistortionTypeParam->choices.size(), 1);
			typeSelector->attach(DistortionTypeParam);

			typeSelector->onChange(typeSelector->getIndex());
			typeSelector->setColorSchemeMinimal(bdsp::NamedColorsIdentifier(), BDSP_COLOR_PURE_BLACK, BDSP_COLOR_DARK, color, BDSP_COLOR_DARK);


			addAndMakeVisible(typeSelector.get());


		}

		void resized() override
		{
			FXUI::resized();

			auto leftW = 0.55;
			auto selectorH = 0.25;

			auto knobBounds = usableRect.getProportion(juce::Rectangle<float>(0, 0, leftW, 1));

			preGainSlider->setBounds(bdsp::shrinkRectangleToInt(knobBounds.getProportion(juce::Rectangle<float>(0, 0, 0.5, 0.5)).reduced(universals->rectThicc / 2).withTrimmedTop(universals->rectThicc / 2)));
			amountSlider->setBounds(bdsp::shrinkRectangleToInt(knobBounds.getProportion(juce::Rectangle<float>(0.5, 0, 0.5, 0.5)).reduced(universals->rectThicc / 2).withTrimmedTop(universals->rectThicc / 2)));

			AGCSection->setBounds(bdsp::shrinkRectangleToInt(knobBounds.getProportion(juce::Rectangle<float>(0, 0.5, 0.5, 0.5)).reduced(universals->rectThicc / 2).withTrimmedBottom(universals->rectThicc / 2)));
			AGCButton->setBounds(preGainSlider->slider.getKnobBounds(true));
			AGCLabel->setBounds(preGainSlider->slider.label.getBounds());

			mixSlider->setBounds(bdsp::shrinkRectangleToInt(knobBounds.getProportion(juce::Rectangle<float>(0.5, 0.5, 0.5, 0.5)).reduced(universals->rectThicc / 2).withTrimmedBottom(universals->rectThicc / 2)));


			visualizer->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(leftW, 0, 1 - leftW, 1 - selectorH)).reduced(universals->rectThicc)));
			typeSelector->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(leftW, 1 - selectorH, 1 - leftW, selectorH)).reduced(universals->rectThicc)));


		}

		float getDesiredAspectRatio() override
		{
			return 2;
		}


	private:
		std::unique_ptr<bdsp::RangedCircleSlider> preGainSlider, amountSlider;
		std::unique_ptr<bdsp::ToggleSlider> AGCButton;
		std::unique_ptr<bdsp::MousePassthrough<bdsp::Label>> AGCLabel;
		std::unique_ptr<bdsp::Component> AGCSection;
		std::unique_ptr<bdsp::DistortionVisualizer> visualizer;
		std::unique_ptr<bdsp::IncrementalComboBox<bdsp::AdvancedComboBox>> typeSelector;
	};

	//================================================================================================================================================================================================
	//Bit Crush

	class BitCrushUI : public FXUI
	{
	public:
		BitCrushUI(FlexFXAudioProcessorEditor* e)
			:FXUI(e, 1)
		{
			auto color = bdsp::NamedColorsIdentifier(e->FXTypeNames->operator[](FlexFXAudioProcessor::BitCrush));



			universals->hintBar->addTextToRecolor({ "Bit Crush", color });
			universals->hintBar->addTextToRecolor({ "Bit Crush's", color });

			//================================================================================================================================================================================================

			createRangedCircleSlider("Bit CrushDepth", depthSlider, color, "Bit Depth");

			depthSlider->setHintBarText("Amplitude resolution of Bit Crush");

			//================================================================================================================================================================================================


			createRangedCircleSlider("Bit CrushRate", rateSlider, color, "Rate");

			rateSlider->setHintBarText("Sampling rate of Bit Crush");

			//================================================================================================================================================================================================

			createMixAndBypass("Bit Crush", color);



			//================================================================================================================================================================================================

			visualizer = std::make_unique< bdsp::BitCrushVisualizer>(universals, &depthSlider->slider, &rateSlider->slider, &mixSlider->slider);
			addAndMakeVisible(visualizer.get());
			visualizer->getVis()->setColor(color);
			visualizer->getVis()->setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_BLACK);
			visualizer->getVis()->setScaling(0.9, 0.9);
		}

		void resized() override
		{

			FXUI::resized();

			depthSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, 0, 0.25, 1)).reduced(universals->rectThicc)));
			rateSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0.25, 0, 0.25, 1)).reduced(universals->rectThicc)));
			mixSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0.5, 0, 0.25, 1)).reduced(universals->rectThicc)));

			visualizer->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0.75, 0, 0.25, 1)).reduced(universals->rectThicc)));

		}

		float getDesiredAspectRatio() override
		{
			return 3.25;
		}


		std::unique_ptr<bdsp::RangedCircleSlider> depthSlider, rateSlider;

		std::unique_ptr<bdsp::BitCrushVisualizer> visualizer;

	};


	//================================================================================================================================================================================================
	//Filter

	class FilterUI : public FXUI
	{
	public:
		FilterUI(FlexFXAudioProcessorEditor* e)
			:FXUI(e, 2)
		{
			auto color = bdsp::NamedColorsIdentifier(e->FXTypeNames->operator[](FlexFXAudioProcessor::Filter));


			universals->hintBar->addTextToRecolor({ "Filter", color });
			universals->hintBar->addTextToRecolor({ "Filter's", color });

			//================================================================================================================================================================================================

			createRangedCircleSlider("Filter Type", typeSlider, color, "Type");

			typeSlider->setHintBarText("Type of Filter");

			//================================================================================================================================================================================================

			createRangedCircleSlider("Filter Frequency", freqSlider, color, "Freq");

			freqSlider->setHintBarText("Cutoff frequency of Filter");

			//================================================================================================================================================================================================

			createRangedCircleSlider("Filter Resonance", QSlider, color, "Res");
			QSlider->setHintBarText("Resonance of Filter");

			//================================================================================================================================================================================================

			createMixAndBypass("Filter", color);

			//================================================================================================================================================================================================

			visualizer = std::make_unique<bdsp::FilterVisualizer>(universals, dynamic_cast<FlexFXAudioProcessor*>(&e->audioProcessor)->filter.get(), &typeSlider->slider, &freqSlider->slider, &QSlider->slider, &mixSlider->slider);
			addAndMakeVisible(visualizer.get());
			visualizer->getVis()->setColor(color);
			visualizer->getVis()->setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_BLACK);

		}

		void resized() override
		{

			FXUI::resized();
			auto topH = 0.5;

			typeSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, 0, 0.25, topH)).withTrimmedTop(universals->rectThicc)));
			freqSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0.25, 0, 0.25, topH)).withTrimmedTop(universals->rectThicc)));
			QSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0.5, 0, 0.25, topH)).withTrimmedTop(universals->rectThicc)));
			mixSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0.75, 0, 0.25, topH)).withTrimmedTop(universals->rectThicc)));


			visualizer->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, topH, 1, 1 - topH)).reduced(universals->rectThicc)));
		}

		float getDesiredAspectRatio() override
		{
			return 2;
		}

		std::unique_ptr<bdsp::RangedCircleSlider> typeSlider, freqSlider, QSlider;

		std::unique_ptr<bdsp::FilterVisualizer> visualizer;

	};

	//================================================================================================================================================================================================
	//EQ

	class EQUI : public FXUI
	{
	public:
		EQUI(FlexFXAudioProcessorEditor* e)
			:FXUI(e, 3)
		{
			auto color = bdsp::NamedColorsIdentifier(e->FXTypeNames->operator[](FlexFXAudioProcessor::ParamEQ));



			universals->hintBar->addTextToRecolor({ "EQ", color });
			universals->hintBar->addTextToRecolor({ "EQ's", color });

			juce::StringArray tabNames;
			tabNames.add("Low");

			for (int j = 1; j < EQNumBands - 1; ++j)
			{
				tabNames.add(juce::String(j));
			}
			tabNames.add("High");

			tabs = std::make_unique< bdsp::TabsComponent>(universals, tabNames);
			tabs->setVertical(false);
			tabs->setTabRatio(0.25);
			tabs->setColors(BDSP_COLOR_DARK, BDSP_COLOR_MID, color, BDSP_COLOR_PURE_BLACK);
			addAndMakeVisible(tabs.get());

			createMixAndBypass("EQ", color);



			createRangedCircleSlider("EQ Global Gain", globalGainSlider, color, "Global");
			globalGainSlider->slider.setHintBarText("Adjust all band's gain values at once");
			globalGainSlider->slider.addCatchValues({ 0 }, false);
			globalGainSlider->slider.setType(bdsp::BaseSlider::CenterZero);

			visualizer = std::make_unique<bdsp::EQVisualizer>(universals, dynamic_cast<FlexFXAudioProcessor*>(&e->audioProcessor)->EQ.get(), 100);

			visualizer->getVis()->setColor(color);

			visualizer->getVis()->setGlobalSliders(&globalGainSlider->slider, &mixSlider->slider);

			visualizer->getVis()->onHandleSelected = [=](int i)
			{
				tabs->selectTab(i);
			};
			tabs->onTabChange = [=](int i)
			{
				visualizer->selectHandle(i);
			};
			addAndMakeVisible(visualizer.get());




			//================================================================================================================================================================================================
			for (int j = 0; j < EQNumBands; ++j)
			{
				auto bandName = "EQ Band " + juce::String(j);


				gainSliders.add(new bdsp::RangedCircleSlider(universals, bandName + " Gain"));
				bdsp::CircleSlider& currentGain = *gainSliders.getLast();





				tabs->getPage(j)->addAndMakeVisible(gainSliders.getLast());
				currentGain.attach(*editor->audioProcessor.parameters.get(), currentGain.getComponentID());

				currentGain.setSliderColor(color, BDSP_COLOR_WHITE);
				currentGain.label.setTitle("Gain");
				currentGain.initText();

				currentGain.setHintBarText("Gain of " + bandName);

				//================================================================================================================================================================================================

				freqSliders.add(new bdsp::RangedCircleSlider(universals, bandName + " Frequency"));
				bdsp::CircleSlider& currentFreq = *freqSliders.getLast();





				tabs->getPage(j)->addAndMakeVisible(freqSliders.getLast());
				currentFreq.attach(*editor->audioProcessor.parameters.get(), currentFreq.getComponentID());

				currentFreq.setSliderColor(color, BDSP_COLOR_WHITE);
				currentFreq.label.setTitle("Freq");
				currentFreq.initText();

				currentFreq.setHintBarText("Frequency of " + bandName);

				//================================================================================================================================================================================================

				BWSLiders.add(new bdsp::RangedCircleSlider(universals, bandName + " Q"));
				bdsp::CircleSlider& currentBW = *BWSLiders.getLast();





				tabs->getPage(j)->addAndMakeVisible(BWSLiders.getLast());
				currentBW.attach(*editor->audioProcessor.parameters.get(), currentBW.getComponentID());

				currentBW.setSliderColor(color, BDSP_COLOR_WHITE);
				currentBW.label.setTitle("Q");
				currentBW.initText();

				currentBW.setHintBarText("Resonance of " + bandName);

				//================================================================================================================================================================================================

				visualizer->getVis()->setSliders(j, &currentFreq, &currentBW, &currentGain);
			}




			//globalGainSlider->boundsPlacement = mixSlider->boundsPlacement = juce::RectanglePlacement::yBottom | juce::RectanglePlacement::xMid;


		}

		void resized() override
		{
			FXUI::resized();
			auto visW = 0.7;
			auto tabH = 0.5;

			tabs->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, 0, 1, tabH)).reduced(universals->rectThicc)));
			globalGainSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, tabH, (1 - visW) / 2, 1 - tabH)).reduced(universals->rectThicc)));
			mixSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>((1 - visW) / 2, tabH, (1 - visW) / 2, 1 - tabH)).reduced(universals->rectThicc)));

			auto tabBounds = tabs->getPage(0)->getLocalBounds();
			for (int i = 0; i < EQNumBands; ++i)
			{
				freqSliders[i]->setBounds(bdsp::shrinkRectangleToInt(tabBounds.getProportion(juce::Rectangle<float>(0, 0, 1.0 / 3.0, 1)).reduced(universals->rectThicc)));
				BWSLiders[i]->setBounds(bdsp::shrinkRectangleToInt(tabBounds.getProportion(juce::Rectangle<float>(1.0 / 3.0, 0, 1.0 / 3.0, 1)).reduced(universals->rectThicc)));
				gainSliders[i]->setBounds(bdsp::shrinkRectangleToInt(tabBounds.getProportion(juce::Rectangle<float>(2.0 / 3.0, 0, 1.0 / 3.0, 1)).reduced(universals->rectThicc)));
			}

			visualizer->getVis()->setHandleSize(2 * universals->visualizerLineThickness, 5 * universals->visualizerLineThickness, universals->visualizerLineThickness);
			visualizer->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(1 - visW, tabH, visW, 1 - tabH)).reduced(universals->rectThicc)));
		}


		float getDesiredAspectRatio() override
		{
			return 1.5;
		}
	private:

		std::unique_ptr<bdsp::TabsComponent> tabs;
		juce::OwnedArray<bdsp::RangedCircleSlider> gainSliders, freqSliders, BWSLiders;
		std::unique_ptr<bdsp::RangedCircleSlider> globalGainSlider;
		std::unique_ptr<bdsp::EQVisualizer> visualizer;
	};

	//================================================================================================================================================================================================
	//PitchShift
	class PitchShiftUI : public FXUI
	{
	public:
		PitchShiftUI(FlexFXAudioProcessorEditor* e)
			:FXUI(e, 4)
		{
			auto color = bdsp::NamedColorsIdentifier(e->FXTypeNames->operator[](FlexFXAudioProcessor::PitchShift));



			universals->hintBar->addTextToRecolor({ "Pitch Shift", color });
			universals->hintBar->addTextToRecolor({ "Pitch Shift's", color });

			//================================================================================================================================================================================================

			createRangedCircleSlider("Pitch Shift Left", leftSlider, color, "L");

			leftSlider->slider.setHintBarText("Shift amount of Pitch Shift's left channel (in semitones)");
			leftSlider->slider.addCatchValues({ -12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12 }, false);


			//================================================================================================================================================================================================
			auto linkDummyName = "Pitch Shift Link Dummy";
			linkDummySlider = std::make_unique<bdsp::RangedCircleSlider>(universals, linkDummyName);
			bdsp::CircleSlider& currentDummy = *linkDummySlider.get();




			addChildComponent(linkDummySlider.get());
			currentDummy.attach(*e->audioProcessor.parameters.get(), leftSlider->slider.getComponentID());

			currentDummy.setSliderColor(color, BDSP_COLOR_WHITE);
			currentDummy.label.setTitle(juce::String("R"));
			currentDummy.initText();


			currentDummy.setHintBarText("Shift amount of Pitch Shift's right channel (in semitones)");

			currentDummy.addCatchValues({ -12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12 }, false);

			leftSlider->addHoverPartner(linkDummySlider.get());

			//================================================================================================================================================================================================




			auto prevLeftShow = leftSlider->hoverMenu->onShow;
			leftSlider->hoverMenu->onShow = [=]()
			{
				prevLeftShow();
				if (linkButton->getToggleState())
				{
					linkDummySlider->hoverMenu->hide();
					linkDummySlider->hoverMenu->resizeMenu();
					leftSlider->hoverMenu->setBounds(leftSlider->hoverMenu->getBounds().withCentre(linkDummySlider->hoverMenu->getBounds().getCentre()));
				}
			};

			auto prevDummyShow = linkDummySlider->hoverMenu->onShow;
			linkDummySlider->hoverMenu->onShow = [=]()
			{
				prevDummyShow();

				leftSlider->hoverMenu->hide();
			};

			for (int j = 0; j < BDSP_NUMBER_OF_LINKABLE_CONTROLS; ++j)
			{
				auto prevLeftFunc = leftSlider->influences[j]->getHoverFunc();

				std::function<void(bool)> leftFunc = [=](bool state)
				{
					prevLeftFunc(state);
					linkDummySlider->displays[j]->setVisible(state);
				};
				leftSlider->influences[j]->setHoverFunc(leftFunc);

				//================================================================================================================================================================================================

				auto prevDummyFunc = linkDummySlider->influences[j]->getHoverFunc();

				std::function<void(bool)> dummyFunc = [=](bool state)
				{
					prevDummyFunc(state);
					leftSlider->displays[j]->setVisible(state);
				};
				linkDummySlider->influences[j]->setHoverFunc(dummyFunc);
			}
			//================================================================================================================================================================================================


			createRangedCircleSlider("Pitch Shift Right", rightSlider, color, "R");

			rightSlider->slider.setHintBarText("Shift amount of Pitch Shift's right channel (in semitones)");
			rightSlider->slider.addCatchValues({ -12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12 }, false);



			//================================================================================================================================================================================================

			createMixAndBypass("Pitch Shift", color);






			//================================================================================================================================================================================================

			visualizer = std::make_unique<bdsp::PitchShifterVisualizer>(universals, leftSlider->getControlParamter(), rightSlider->getControlParamter(), mixSlider->getControlParamter());
			addAndMakeVisible(visualizer.get());
			visualizer->getVis()->setColor(color);
			visualizer->getVis()->setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_BLACK);



			//================================================================================================================================================================================================

			linkButton = std::make_unique<bdsp::PathButton>(universals, true, false);

			addAndMakeVisible(linkButton.get());

			linkButton->setPath(universals->commonPaths.linkSymbol);
			linkButton->setColor(BDSP_COLOR_KNOB);




			linkButton->onStateChange = [=]()
			{
				auto on = linkButton->getToggleState();

				rightSlider->setVisible(!on);
				linkDummySlider->setVisible(on);

				visualizer->getVis()->linkAmounts(on);

				linkButton->setHintBarText(juce::String(on ? "Unlinks" : "Links") + " Pitch Shift's left and right channel's");
			};

			linkButton->attach(*e->audioProcessor.parameters.get(), "PitchShiftLinkID");
		}

		void resized() override
		{
			FXUI::resized();

			auto knobW = 0.6;
			auto buttonH = 0.25;

			leftSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, 0, knobW / 2, 0.5)).reduced(universals->rectThicc)));
			rightSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(knobW / 2, 0, knobW / 2, 0.5)).reduced(universals->rectThicc)));
			mixSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, 0.5, knobW, 0.5)).reduced(universals->rectThicc)));

			auto linkButtonBounds = bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(knobW / 2, 0, knobW / 2, buttonH)).reduced(universals->rectThicc));
			linkButton->setBounds(linkButtonBounds.withCentre({ (leftSlider->getRight() + rightSlider->getX()) / 2 , leftSlider->getY() + leftSlider->slider.getKnobBounds(true).getCentreY() }));

			linkDummySlider->setBounds(rightSlider->getBounds());

			visualizer->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(knobW, 0, 1 - knobW, 1)).reduced(universals->rectThicc)));

		}

		float getDesiredAspectRatio() override
		{
			return 2.25;
		}



	private:

		std::unique_ptr<bdsp::RangedCircleSlider> leftSlider, rightSlider, linkDummySlider;
		std::unique_ptr<bdsp::PathButton> linkButton;

		std::unique_ptr<bdsp::PitchShifterVisualizer> visualizer;
	};

	//================================================================================================================================================================================================
	//RingMod
	class RingModUI : public FXUI
	{
	public:
		RingModUI(FlexFXAudioProcessorEditor* e)
			:FXUI(e, 5)
		{
			auto color = bdsp::NamedColorsIdentifier(e->FXTypeNames->operator[](FlexFXAudioProcessor::RingMod));


			universals->hintBar->addTextToRecolor({ "Ring Mod", color });
			universals->hintBar->addTextToRecolor({ "Ring Mod's", color });

			//================================================================================================================================================================================================


			createRangedCircleSlider("Ring Mod Shape", shapeSlider, color, "Shape");

			shapeSlider->slider.setHintBarText("Shape of Ring Mod");

			shapeSlider->slider.addCatchValues({ 0.5 });

			//================================================================================================================================================================================================

			createRangedCircleSlider("Ring Mod Skew", skewSlider, color, "Skew");

			skewSlider->slider.setHintBarText("Skew of Ring Mod");
			skewSlider->slider.addCatchValues({ 0.5 });

			//===============================================================================================================================================================================================

			createRangedCircleSlider("Ring Mod Frequency", freqSlider, color, "Freq");

			freqSlider->slider.setHintBarText("Frequency of Ring Mod");

			//===============================================================================================================================================================================================
			createMixAndBypass("Ring Mod", color);


			//===============================================================================================================================================================================================

			auto sourceParam = dynamic_cast<juce::AudioParameterChoice*>(e->audioProcessor.parameters->getParameter("RingModSourceID"));
			sourceCombo = std::make_unique< bdsp::ComboBox>(sourceParam, universals);
			addAndMakeVisible(sourceCombo.get());

			sourceCombo->onChange = [=](int i)
			{
				shapeSlider->setEnabled(i == 0);
				skewSlider->setEnabled(i == 0);
				freqSlider->setEnabled(i == 0);

				visualizer->getVis()->setSource(bdsp::dsp::RingModulation<float>::RingModSource(i + 1));
			};

			sourceLabel = std::make_unique<bdsp::Label>(universals);
			addAndMakeVisible(sourceLabel.get());
			sourceLabel->setColour(juce::Label::textColourId, getColor(color));

			sourceLabel->setText("Mod Source", juce::sendNotification);

			//================================================================================================================================================================================================
			visualizer = std::make_unique<bdsp::RingModVisualizer>(universals, &e->audioProcessor.lookups, e->sampleRate);
			addAndMakeVisible(visualizer.get());
			auto currentVis = visualizer->getVis();
			currentVis->setColor(color, BDSP_COLOR_DARK);
			currentVis->setScaling(0.95, 0.9);

			currentVis->shapeParam = shapeSlider->getControlParamter();
			currentVis->skewParam = skewSlider->getControlParamter();
			currentVis->freqParam = freqSlider->getControlParamter();
			currentVis->mixParam = mixSlider->getControlParamter();

			sourceCombo->onChange(sourceCombo->getIndex());

		}

		void resized() override
		{
			FXUI::resized();
			auto visW = 0.5;

			mixSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(2.0 / 3.0, 0, 1.0 / 3.0, 0.5)).reduced(universals->rectThicc)));
			auto sourceRect = juce::Rectangle<float>(0, mixSlider->getY() + mixSlider->slider.getKnobBounds(true).getY(), mixSlider->getX(), mixSlider->slider.getKnobBounds(true).getHeight());
			sourceCombo->setBounds(bdsp::shrinkRectangleToInt(sourceRect.reduced(2 * universals->rectThicc, 0)));
			sourceLabel->setBounds(juce::Rectangle<int>().leftTopRightBottom(sourceCombo->getX(), mixSlider->getY() + mixSlider->slider.getTrackBounds(true).getBottom(), sourceCombo->getRight(), mixSlider->getBottom()));


			freqSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(visW, 0.5, (1 - visW) / 3.0, 0.5)).reduced(universals->rectThicc)));
			shapeSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(visW + (1 - visW) / 3.0, 0.5, (1 - visW) / 3.0, 0.5)).reduced(universals->rectThicc)));
			skewSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(visW + 2 * (1 - visW) / 3.0, 0.5, (1 - visW) / 3.0, 0.5)).reduced(universals->rectThicc)));

			visualizer->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, 0.5, visW, 0.5)).reduced(universals->rectThicc)));

		}

		float getDesiredAspectRatio() override
		{
			return 2.0;
		}
	private:

		std::unique_ptr<bdsp::RangedCircleSlider> shapeSlider, skewSlider, freqSlider;
		std::unique_ptr<bdsp::ComboBox> sourceCombo;
		std::unique_ptr<bdsp::Label> sourceLabel;
		std::unique_ptr<bdsp::RingModVisualizer> visualizer;
	};

	//================================================================================================================================================================================================
	//Chorus
	class ChorusUI : public FXUI
	{
	public:
		ChorusUI(FlexFXAudioProcessorEditor* e)
			:FXUI(e, 6)
		{
			auto color = bdsp::NamedColorsIdentifier(e->FXTypeNames->operator[](FlexFXAudioProcessor::Chorus));



			universals->hintBar->addTextToRecolor({ "Chorus", color });
			universals->hintBar->addTextToRecolor({ "Chorus's", color });

			//================================================================================================================================================================================================

			auto divParam = dynamic_cast<juce::AudioParameterChoice*>(e->audioProcessor.parameters->getParameter("ChorusRateDivisionID"));
			rateFrac = std::make_unique<bdsp::RangedSyncFraction>(divParam, universals, e->topLevelComp->BPM.get(), "ChorusRate", true);
			rateFrac->setFracColors(BDSP_COLOR_KNOB, color, BDSP_COLOR_DARK, BDSP_COLOR_LIGHT, color, color, color, color, color.withMultipliedAlpha(universals->lowOpacity));
			rateFrac->getDivision()->setColorSchemeClassic(BDSP_COLOR_KNOB, bdsp::NamedColorsIdentifier(), bdsp::NamedColorsIdentifier(BDSP_COLOR_BLACK), color.withMultipliedAlpha(universals->lowOpacity));
			rateFrac->attach(*e->audioProcessor.parameters.get(), "ChorusRate");

			addAndMakeVisible(rateFrac.get());

			rateFrac->setSpacing(0, 0.5, 0.5, 0.5, juce::RectanglePlacement::xLeft | juce::RectanglePlacement::yMid);

			//================================================================================================================================================================================================

			createRangedCircleSlider("Chorus Depth", depthSlider, color, "Depth");

			depthSlider->setHintBarText("Intensity of Chorus");

			//================================================================================================================================================================================================

			createRangedCircleSlider("Chorus Stereo", stereoWidthSlider, color, "Stereo");
			stereoWidthSlider->setHintBarText("Stereo width of Chorus");


			//================================================================================================================================================================================================
			auto voiceName = "Chorus Voices";
			voiceSlider = std::make_unique<bdsp::CircleSlider>(universals, voiceName);
			voiceSlider->attach(*e->audioProcessor.parameters.get(), voiceSlider->getComponentID());

			voiceSlider->setSliderColor(color, BDSP_COLOR_WHITE);
			voiceSlider->label.setTitle(juce::String("Voices"));
			voiceSlider->initText();

			voiceSlider->setHintBarText("Chorus's number of voices");
			addAndMakeVisible(voiceSlider.get());



			//================================================================================================================================================================================================

			createMixAndBypass("Chorus", color);

			//================================================================================================================================================================================================
			visualizer = std::make_unique<bdsp::ChorusVisualizer>(universals, dynamic_cast<FlexFXAudioProcessor*>(&e->audioProcessor)->chorus.get(), &e->audioProcessor.lookups);
			addAndMakeVisible(visualizer.get());
			visualizer->getVis()->setColor(color);
			visualizer->getVis()->setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_BLACK);

			visualizer->getVis()->depthParam = depthSlider->getControlParamter();
			visualizer->getVis()->stereoWidthParam = stereoWidthSlider->getControlParamter();
			visualizer->getVis()->mixParam = mixSlider->getControlParamter();
			visualizer->getVis()->numVoicesParam = voiceSlider->getParameter();

		}

		void resized() override
		{
			FXUI::resized();
			auto row1 = 0.35;
			auto row2 = 0.35;
			auto visH = 0.3;

			rateFrac->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, 0, 2.0 / 3.0, row1)).withTrimmedTop(universals->rectThicc)));
			mixSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(2.0 / 3.0, 0, 1.0 / 3.0, row1)).withTrimmedTop(universals->rectThicc)));

			depthSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, row1, 1.0 / 3.0, row2)).withTrimmedTop(universals->rectThicc)));
			stereoWidthSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(1.0 / 3.0, row1, 1.0 / 3.0, row2)).withTrimmedTop(universals->rectThicc)));
			voiceSlider->setBoundsToIncludeLabel(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(2.0 / 3.0, row1, 1.0 / 3.0, row2)).withTrimmedTop(universals->rectThicc)));

			visualizer->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, row1 + row2, 1, visH)).reduced(universals->rectThicc)));

		}

		float getDesiredAspectRatio() override
		{
			return 1.618;
		}

	private:

		std::unique_ptr<bdsp::RangedCircleSlider> depthSlider, stereoWidthSlider;
		std::unique_ptr<bdsp::RangedSyncFraction> rateFrac;
		std::unique_ptr<bdsp::CircleSlider> voiceSlider;
		std::unique_ptr<bdsp::ChorusVisualizer> visualizer;

	};



	//================================================================================================================================================================================================
	//Flanger
	class FlangerUI : public FXUI
	{
	public:
		FlangerUI(FlexFXAudioProcessorEditor* e)
			:FXUI(e, 7)
		{
			auto color = bdsp::NamedColorsIdentifier(e->FXTypeNames->operator[](FlexFXAudioProcessor::Flanger));


			universals->hintBar->addTextToRecolor({ "Flanger", color });
			universals->hintBar->addTextToRecolor({ "Flanger's", color });

			//================================================================================================================================================================================================

			auto divParam = dynamic_cast<juce::AudioParameterChoice*>(e->audioProcessor.parameters->getParameter("FlangerRateDivisionID"));
			rateFrac = std::make_unique<bdsp::RangedSyncFraction>(divParam, universals, e->topLevelComp->BPM.get(), "FlangerRate", true);
			rateFrac->setFracColors(BDSP_COLOR_KNOB, color, BDSP_COLOR_DARK, BDSP_COLOR_LIGHT, color, color, color, color, color.withMultipliedAlpha(universals->lowOpacity));
			rateFrac->getDivision()->setColorSchemeClassic(BDSP_COLOR_KNOB, bdsp::NamedColorsIdentifier(), bdsp::NamedColorsIdentifier(BDSP_COLOR_BLACK), color.withMultipliedAlpha(universals->lowOpacity));

			rateFrac->attach(*e->audioProcessor.parameters.get(), "FlangerRate");

			addAndMakeVisible(rateFrac.get());

			rateFrac->setSpacing(0, 0.5, 0.5, 0.5, juce::RectanglePlacement::xLeft | juce::RectanglePlacement::yMid);

			//================================================================================================================================================================================================
			auto baseName = "Flanger Base";

			createRangedCircleSlider("Flanger Base", baseSlider, color, "Base");

			baseSlider->setHintBarText("Base delay of Flanger");
			//================================================================================================================================================================================================

			createRangedCircleSlider("Flanger Depth", depthSlider, color, "Depth");

			depthSlider->setHintBarText("Depth of Flanger's delay change");

			//================================================================================================================================================================================================
			createRangedCircleSlider("Flanger Stereo", stereoWidthSlider, color, "Stereo");

			stereoWidthSlider->setHintBarText("Stereo width of Flanger");

			//================================================================================================================================================================================================

			createRangedCircleSlider("Flanger Feedback", feedbackSlider, color, "FB");

			feedbackSlider->slider.addCatchValues({ 0.5 });

			feedbackSlider->slider.setType(bdsp::BaseSlider::CenterZero);


			feedbackSlider->setHintBarText("Feedback of Flanger");

			//================================================================================================================================================================================================

			createMixAndBypass("Flanger", color);

			//================================================================================================================================================================================================

			visualizer = std::make_unique<bdsp::FlangerVisualizer>(universals, dynamic_cast<FlexFXAudioProcessor*>(&e->audioProcessor)->flanger.get(), &e->audioProcessor.lookups);
			addAndMakeVisible(visualizer.get());
			visualizer->getVis()->setColor(color);
			visualizer->getVis()->setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_BLACK);

			visualizer->getVis()->feedbackParam = feedbackSlider->getControlParamter();
			visualizer->getVis()->mixParam = mixSlider->getControlParamter();
		}

		void resized() override
		{

			FXUI::resized();
			auto row1 = 0.35;
			auto row2 = 0.35;
			auto visH = 0.3;

			rateFrac->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, 0, 2.0 / 3.0, row1)).withTrimmedTop(universals->rectThicc)));
			mixSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(2.0 / 3.0, 0, 1.0 / 3.0, row1)).withTrimmedTop(universals->rectThicc)));


			baseSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, row1, 1.0 / 4.0, row2)).withTrimmedTop(universals->rectThicc)));
			depthSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(1.0 / 4.0, row1, 1.0 / 4.0, row2)).withTrimmedTop(universals->rectThicc)));
			feedbackSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(2.0 / 4.0, row1, 1.0 / 4.0, row2)).withTrimmedTop(universals->rectThicc)));
			stereoWidthSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(3.0 / 4.0, row1, 1.0 / 4.0, row2)).withTrimmedTop(universals->rectThicc)));

			visualizer->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, row1 + row2, 1, visH)).reduced(universals->rectThicc)));

		}

		float getDesiredAspectRatio() override
		{
			return 1.618;
		}
	private:


		std::unique_ptr<bdsp::RangedCircleSlider> baseSlider, depthSlider, stereoWidthSlider, feedbackSlider;
		std::unique_ptr<bdsp::RangedSyncFraction> rateFrac;
		std::unique_ptr<bdsp::FlangerVisualizer> visualizer;

	};


	//================================================================================================================================================================================================
	//Phaser

	class PhaserUI : public FXUI
	{
	public:
		PhaserUI(FlexFXAudioProcessorEditor* e)
			:FXUI(e, 8)
		{

			auto color = bdsp::NamedColorsIdentifier(e->FXTypeNames->operator[](FlexFXAudioProcessor::Phaser));



			universals->hintBar->addTextToRecolor({ "Phaser", color });
			universals->hintBar->addTextToRecolor({ "Phaser's", color });

			//================================================================================================================================================================================================

			auto divParam = dynamic_cast<juce::AudioParameterChoice*>(e->audioProcessor.parameters->getParameter("PhaserRateDivisionID"));
			rateFrac = std::make_unique<bdsp::RangedSyncFraction>(divParam, universals, e->topLevelComp->BPM.get(), "PhaserRate", true);
			rateFrac->setFracColors(BDSP_COLOR_KNOB, color, BDSP_COLOR_DARK, BDSP_COLOR_LIGHT, color, color, color, color, color.withMultipliedAlpha(universals->lowOpacity));
			rateFrac->getDivision()->setColorSchemeClassic(BDSP_COLOR_KNOB, bdsp::NamedColorsIdentifier(), bdsp::NamedColorsIdentifier(BDSP_COLOR_BLACK), color.withMultipliedAlpha(universals->lowOpacity));

			rateFrac->attach(*e->audioProcessor.parameters.get(), "PhaserRate");

			addAndMakeVisible(rateFrac.get());

			rateFrac->setSpacing(0, 0.5, 0.5, 0.5, juce::RectanglePlacement::xLeft | juce::RectanglePlacement::yMid);
			//================================================================================================================================================================================================

			createRangedCircleSlider("Phaser Center", centerSlider, color, "Center");

			centerSlider->setHintBarText("Center phase of Phaser");

			//================================================================================================================================================================================================

			createRangedCircleSlider("Phaser Depth", depthSlider, color, "Depth");

			depthSlider->setHintBarText("Depth of Phaser's delay change");

			//================================================================================================================================================================================================

			createRangedCircleSlider("Phaser Stereo", stereoWidthSlider, color, "Stereo");

			stereoWidthSlider->setHintBarText("Stereo width of Phaser");

			//================================================================================================================================================================================================


			createRangedCircleSlider("Phaser Feedback", feedbackSlider, color, "FB");

			feedbackSlider->slider.addCatchValues({ 0.5 });

			feedbackSlider->setHintBarText("Feedback of Phaser");



			//================================================================================================================================================================================================

			createMixAndBypass("Phaser", color);


			//================================================================================================================================================================================================

			visualizer = std::make_unique< bdsp::PhaserVisualizer>(universals, dynamic_cast<FlexFXAudioProcessor*>(&e->audioProcessor)->phaser.get(), &e->audioProcessor.lookups);
			addAndMakeVisible(visualizer.get());
			visualizer->getVis()->setColor(color);
			visualizer->getVis()->setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_BLACK);

			visualizer->getVis()->centerParam = centerSlider->getControlParamter();
			visualizer->getVis()->depthParam = depthSlider->getControlParamter();
			visualizer->getVis()->stereoWidthParam = stereoWidthSlider->getControlParamter();
			visualizer->getVis()->feedbackParam = feedbackSlider->getControlParamter();
			visualizer->getVis()->mixParam = mixSlider->getControlParamter();

		}



		void resized() override
		{
			FXUI::resized();
			auto row1 = 0.35;
			auto row2 = 0.35;
			auto visH = 0.3;

			rateFrac->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, 0, 2.0 / 3.0, row1)).withTrimmedTop(universals->rectThicc)));
			mixSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(2.0 / 3.0, 0, 1.0 / 3.0, row1)).withTrimmedTop(universals->rectThicc)));

			centerSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, row1, 0.25, row2)).withTrimmedTop(universals->rectThicc)));
			depthSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0.25, row1, 0.25, row2)).withTrimmedTop(universals->rectThicc)));

			stereoWidthSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0.5, row1, 0.25, row2)).withTrimmedTop(universals->rectThicc)));
			feedbackSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0.75, row1, 0.25, row2)).withTrimmedTop(universals->rectThicc)));

			visualizer->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, row1 + row2, 1, visH)).reduced(universals->rectThicc)));
		}

		float getDesiredAspectRatio() override
		{
			return 1.618;
		}
	private:

		std::unique_ptr<bdsp::RangedCircleSlider> centerSlider, depthSlider, stereoWidthSlider, feedbackSlider;
		std::unique_ptr<bdsp::RangedSyncFraction> rateFrac;
		std::unique_ptr<bdsp::PhaserVisualizer> visualizer;
	};

	//================================================================================================================================================================================================
	//Panner

	class PannerUI : public FXUI
	{
	public:
		PannerUI(FlexFXAudioProcessorEditor* e)
			:FXUI(e, 9)
		{
			auto color = bdsp::NamedColorsIdentifier(e->FXTypeNames->operator[](FlexFXAudioProcessor::Panner));


			universals->hintBar->addTextToRecolor({ "Panner", color });
			universals->hintBar->addTextToRecolor({ "Panner's", color });

			//================================================================================================================================================================================================

			createRangedCircleSlider("Panner Gain", gainSlider, color, "Gain");

			gainSlider->setHintBarText("Gain of Panner");

			//================================================================================================================================================================================================

			createRangedCircleSlider("Panner Pan", panSlider, color, "Pan");

			panSlider->setHintBarText("Pan of Panner");

			//================================================================================================================================================================================================

			createRangedCircleSlider("Panner Stereo Width", widthSlider, color, "Stereo");

			widthSlider->setHintBarText("Stereo width of Panner");




			//================================================================================================================================================================================================

			createMixAndBypass("Panner", color);

			//================================================================================================================================================================================================

			meter = std::make_unique< bdsp::LissajousMeter<float>>(universals, dynamic_cast<FlexFXAudioProcessor*>(&e->audioProcessor)->pannerMeterSource.get());
			addAndMakeVisible(meter.get());
			meter->getVis()->setColors(color);
			meter->getVis()->setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_BLACK);

			//================================================================================================================================================================================================




		}



		void resized() override
		{
			FXUI::resized();

			auto visW = usableRect.getHeight();
			meter->setBounds(juce::Rectangle<int>(usableRect.getRight() - visW, usableRect.getY(), visW, visW).reduced(universals->rectThicc));

			auto w = 1 - visW / getWidth();


			gainSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, 0, w / 4, 1)).reduced(universals->rectThicc)));
			panSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(w / 4, 0, w / 4, 1)).reduced(universals->rectThicc)));
			widthSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(w / 2, 0, w / 4, 1)).reduced(universals->rectThicc)));
			mixSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(3 * w / 4, 0, w / 4, 1)).reduced(universals->rectThicc)));


		}

		float getDesiredAspectRatio() override
		{
			return 3.25;
		}
	private:

		std::unique_ptr<bdsp::Component> pannerSections;

		std::unique_ptr<bdsp::RangedCircleSlider> gainSlider, panSlider, widthSlider;
		std::unique_ptr<bdsp::LissajousMeter<float>> meter;

	};

	//================================================================================================================================================================================================
	//Noise
	class NoiseUI : public FXUI
	{
	public:
		NoiseUI(FlexFXAudioProcessorEditor* e)
			:FXUI(e, 10)
		{

			auto color = bdsp::NamedColorsIdentifier(e->FXTypeNames->operator[](FlexFXAudioProcessor::Noise));



			universals->hintBar->addTextToRecolor({ "Noise", color });
			universals->hintBar->addTextToRecolor({ "Noise's", color });

			//================================================================================================================================================================================================

			createRangedCircleSlider("Noise Gain", gainSlider, color, "Gain");

			gainSlider->slider.setHintBarText("Gain of Noise");

			//================================================================================================================================================================================================

			createRangedCircleSlider("Noise Color", colorSlider, color, "Color");

			colorSlider->setHintBarText("Color of Noise");
			colorSlider->slider.addCatchValues({ 0 }, false);
			colorSlider->slider.setType(bdsp::BaseSlider::CenterZero);

			//================================================================================================================================================================================================

			createRangedCircleSlider("Noise Stereo", stereoSlider, color, "Stereo");

			stereoSlider->slider.setHintBarText("Stereo Width of Noise");

			//================================================================================================================================================================================================

			createRangedCircleSlider("Noise Dry Gain", drySlider, color, "Dry Gain");

			drySlider->setHintBarText("Gain of input signal");

			//================================================================================================================================================================================================

			createMixAndBypass("Noise", color);

			//================================================================================================================================================================================================

			visualizer = std::make_unique<bdsp::NoiseVisualizer>(universals);
			visualizer->getVis()->gainParam = gainSlider->getControlParamter();
			visualizer->getVis()->colorParam = colorSlider->getControlParamter();

			visualizer->getVis()->setColor(color);

			addAndMakeVisible(visualizer.get());


		}



		void resized() override
		{
			FXUI::resized();

			auto visW = 0.4;
			auto w = 1 - visW;

			gainSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(0, 0, w / 4, 1)).reduced(universals->rectThicc)));
			colorSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(w / 4, 0, w / 4, 1)).reduced(universals->rectThicc)));
			stereoSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(w / 2, 0, w / 4, 1)).reduced(universals->rectThicc)));
			mixSlider->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(3 * w / 4, 0, w / 4, 1)).reduced(universals->rectThicc)));

			visualizer->setBounds(bdsp::shrinkRectangleToInt(usableRect.getProportion(juce::Rectangle<float>(w, 0, visW, 1)).reduced(universals->rectThicc)));

		}

		float getDesiredAspectRatio() override
		{
			return 3.25;
		}
	private:


		std::unique_ptr<bdsp::RangedCircleSlider> gainSlider, colorSlider, stereoSlider, drySlider;
		std::unique_ptr<bdsp::NoiseVisualizer> visualizer;

	};


	//================================================================================================================================================================================================

	int maxWidth = 1500;

	class FXSlot : public bdsp::Component, public bdsp::GUI_Universals::Listener
	{
	public:
		FXSlot(FlexFXAudioProcessorEditor* editor, FlexFXAudioProcessor::FXTypes fxType);
		virtual ~FXSlot() override
		{

		}

		void paint(juce::Graphics& g) override;
		void resized() override;



		int getSlotIndex() const;
		void setSlotIndex(int newIdx);


		void GUI_UniversalsChanged() override;

		juce::Rectangle<float> getDraghandleRect();

		FXUI* getFxSection();

		bdsp::NamedColorsIdentifier color;

	private:

		std::unique_ptr<FXUI> FXSection;

		FlexFXAudioProcessorEditor* p = nullptr;

		int slotIndex;
		float border = 0;

		std::unique_ptr<bdsp::TextButton>* bypassButton = nullptr;
	};


	class FXChainManager : public bdsp::RearrangableComponentManagerBase, public juce::ComponentListener
	{
	public:

		FXChainManager(FlexFXAudioProcessorEditor* editor);
		void resized() override;
		void paint(juce::Graphics& g) override;



		void setDragColor(int idx, const bdsp::NamedColorsIdentifier& c)
		{
			setSingleDragHandleColors(idx, c, c, BDSP_COLOR_PURE_BLACK);

		}
		void setDragText(int idx, const juce::String& s)
		{
			setSingleDragHandleText(idx, s);
		}

	private:

		juce::Array<FXSlot*> fxSlots;

		bdsp::OrderedListParameter* orderState = nullptr;

		FlexFXAudioProcessorEditor* p = nullptr;

		bdsp::Viewport vp;
		bdsp::Component vpComp;

		void componentMovedOrResized(juce::Component& component, bool wasMoved, bool wasResized) override
		{
			if (!vp.getViewArea().isEmpty())
			{
				for (auto c : fxSlots)
				{
					bool visible = vp.getViewArea().intersects(c->getBounds());
					c->getFxSection()->setVisible(visible);
				}
			}

		}


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

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlexFXAudioProcessorEditor)







};

