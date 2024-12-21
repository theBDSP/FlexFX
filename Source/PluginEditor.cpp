

#include "PluginEditor.h"


constexpr auto TopBarH = 0.08;
constexpr auto ModH = 0.25;
constexpr auto FXH = 0.525;
constexpr auto MacroH = 0.1;


constexpr auto aspecRatioClosed = 1.3;
constexpr auto aspectRatioOpen = 2.0;


//==============================================================================
FlexFXAudioProcessorEditor::FlexFXAudioProcessorEditor(FlexFXAudioProcessor& p)
	: bdsp::Editor(p)
{

	FXTypeNames = &p.FXTypeNames;




	setBufferedToImage(true);


	init();

	titleBar = std::make_unique<bdsp::TexturedContainerComponent>(&GUIUniversals, BDSP_COLOR_DARK, bdsp::NamedColorsIdentifier(), true);

	titleBar->addAndMakeVisible(topLevelComp->preset->titleBar);
	titleBar->addAndMakeVisible(topLevelComp->undoRedo.get());
	titleBar->addAndMakeVisible(topLevelComp->settingsButton);



	title = std::make_unique<AnimatedTitle>(this);

	loadSettings();

	topLevelComp->settings->setFontIndex(0);



	GUIUniversals.hintBar->setFont(GUIUniversals.Fonts[2].getFont());
	topLevelComp->preset->pc.tagMenuBKGD = bdsp::NamedColorsIdentifier();
	topLevelComp->preset->pc.browserBKGD = bdsp::NamedColorsIdentifier();



	topLevelComp->preset->pc.saveMenuBKGDTop = BDSP_COLOR_BLACK;
	topLevelComp->preset->pc.saveMenuBKGDBot = BDSP_COLOR_BLACK;
	topLevelComp->preset->pc.saveMenuBKGD = BDSP_COLOR_DARK;
	topLevelComp->preset->pc.outline = BDSP_COLOR_MID;

	topLevelComp->preset->pc.searchBarBKGD = BDSP_COLOR_MID;

	topLevelComp->preset->titleBarPopup->setFontIndex(1);
	topLevelComp->preset->menu->Popup->setFontIndex(1);
	topLevelComp->preset->pc.menuPopupWidthRatio = 0.5;

	topLevelComp->preset->menu->setItemFontIndex(1);
	topLevelComp->preset->menu->itemHRatio = 0.0875;


	//================================================================================================================================================================================================

	GUIUniversals.sliderHoverMenuBGKD = BDSP_COLOR_PURE_BLACK;
	GUIUniversals.sliderHoverMenuOutline = BDSP_COLOR_DARK;





	topLevelComp->Alert->setRelativeSize(0.75f, 0.6f);
	topLevelComp->Alert->setFonts(2, 1);


	topLevelComp->aspectRatio = aspecRatioClosed;

	topLevelComp->sidebarAspectRatio = aspectRatioOpen;



	modTabs = std::make_unique<bdsp::TabsComponent>(&GUIUniversals, juce::StringArray({ "LFOs","ENVs" }));
	modTabs->getPage(0)->addAndMakeVisible(LFOs);
	modTabs->getPage(1)->addAndMakeVisible(EnvelopeFollowers);

	modTabs->setVertical(true);
	modTabs->setCorners(bdsp::CornerCurves(bdsp::CornerCurves::bottomLeft | bdsp::CornerCurves::bottomRight));
	modTabs->setColors(BDSP_COLOR_BLACK, BDSP_COLOR_MID, BDSP_COLOR_WHITE);

	modTabs->setTabRatio(0.075);

	topLevelComp->addComponent(Macros);
	topLevelComp->addComponent(modTabs.get());


	LFOs->setColors(bdsp::NamedColorsIdentifier());
	EnvelopeFollowers->setColors(bdsp::NamedColorsIdentifier());
	Macros->setColors(bdsp::NamedColorsIdentifier());
	Macros->shouldDrawDivider = false;

	Macros->setTitleFontIndex(1);
	LFOs->setTitleFontIndex(1);
	EnvelopeFollowers->setTitleFontIndex(1);


	for (int i = 0; i < BDSP_NUMBER_OF_ENVELOPE_FOLLOWERS; ++i)
	{
		EnvelopeFollowers->getFollower(i)->getSource()->setMenuTextHRatio(0.75);
	}

	//================================================================================================================================================================================================


	topLevelComp->addComponent(titleBar.get());


	//================================================================================================================================================================================================

	GUIUniversals.hintBar->setFont(GUIUniversals.Fonts[1].getFont());

	//================================================================================================================================================================================================
	fxSlotManager = std::make_unique<FXChainManager>(this);
	fxSlotManager->setNumCompsPerTab(numFXPerChain);
	fxSlotManager->setColors(BDSP_COLOR_BLACK, BDSP_COLOR_MID, BDSP_COLOR_WHITE);


	topLevelComp->addComponent(fxSlotManager.get());

	for (int i = 0; i < numFXSlots; ++i)
	{
		fxSlotManager->addComponent(i);
		auto cast = dynamic_cast<FXSlot*>(fxSlotManager->getComp(i));
		cast->fxTypeSelector->onChange(cast->fxTypeSelector->getIndex());


	}



	fxSlotManager->setTabTitleHeightRatio(0.5);



	//================================================================================================================================================================================================

	initFX();

	//==========================================================================================================================================================================



	topLevelComp->preset->setFontIndex(1, 0.25);





	float w = p.settingsTree.getProperty("WindowWidth");
	w = w == 0 ? BDSP_DEFAULT_PLUGIN_WIDTH : w;
	w *= (audioProcessor.settings.get("UI Scale") - 2) * 0.25 + 1;

	boundsConstrainer.setFixedAspectRatio(topLevelComp->aspectRatio);
	boundsConstrainer.setSizeLimits(100, 100, 3500, 2000);

	setSize(w, w / topLevelComp->aspectRatio);

	//================================================================================================================================================================================================

	topLevelComp->paintOverChildrenFunction = [=](juce::Graphics& g)
	{
		bdsp::drawDivider(g, juce::Line<float>(modArea.getRelativePoint(0.0, 0.0), modArea.getRelativePoint(1.0, 0.0)), GUIUniversals.colors.getColor(BDSP_COLOR_MID), GUIUniversals.dividerSize);
		bdsp::drawDivider(g, juce::Line<float>(macroArea.getRelativePoint(0.0, 0.0), macroArea.getRelativePoint(0.0, 1.0)), GUIUniversals.colors.getColor(BDSP_COLOR_MID), GUIUniversals.dividerSize);

		auto modRect = modTabs->getUsableArea().toFloat() + modTabs->getPosition().toFloat();
		bdsp::drawDivider(g, juce::Line<float>(modArea.getRelativePoint(0.0, 0.5), modRect.getRelativePoint(0.0, 0.5)), GUIUniversals.colors.getColor(BDSP_COLOR_MID), GUIUniversals.dividerSize);
		bdsp::drawDivider(g, juce::Line<float>(modRect.getRelativePoint(0.0, 0.0), modRect.getRelativePoint(0.0, 1.0)), GUIUniversals.colors.getColor(BDSP_COLOR_MID), GUIUniversals.dividerSize);
		bdsp::drawDivider(g, juce::Line<float>(modRect.getRelativePoint(0.5, 0.0), modRect.getRelativePoint(0.5, 1.0)), GUIUniversals.colors.getColor(BDSP_COLOR_MID), GUIUniversals.dividerSize);


		g.setColour(GUIUniversals.colors.getColor(BDSP_COLOR_PURE_BLACK));
		g.drawRoundedRectangle(topBarArea.reduced(GUIUniversals.dividerSize), GUIUniversals.roundedRectangleCurve, GUIUniversals.dividerSize);
		g.drawRoundedRectangle(juce::Rectangle<float>().leftTopRightBottom(modTabs->getX(), macroArea.getY(), macroArea.getRight(), modTabs->getBottom()).reduced(GUIUniversals.dividerSize), GUIUniversals.roundedRectangleCurve, GUIUniversals.dividerSize);


		g.setColour(GUIUniversals.colors.getColor(BDSP_COLOR_MID));
		g.drawRoundedRectangle(topBarArea.expanded(GUIUniversals.dividerSize / 3), GUIUniversals.roundedRectangleCurve, GUIUniversals.dividerSize / 3);
		g.drawRoundedRectangle(juce::Rectangle<float>().leftTopRightBottom(modTabs->getX(), macroArea.getY(), macroArea.getRight(), modTabs->getBottom()).expanded(GUIUniversals.dividerSize / 3), GUIUniversals.roundedRectangleCurve, GUIUniversals.dividerSize / 3);

	};



	//================================================================================================================================================================================================


	startOpenGlAnimation();
}



FlexFXAudioProcessorEditor::~FlexFXAudioProcessorEditor()
{

	contextHolder.detach();

	Macros->removeMouseListener(this);


	Macros->setLookAndFeel(nullptr);

	corner.setLookAndFeel(nullptr);



	audioProcessor.updateUIProperties(topLevelComp->mainArea.getWidth() / ((audioProcessor.settings.get("UI Scale") - 2) * 0.25 + 1));


	//shutdownOpenGlAnimation();
}

//==============================================================================
void FlexFXAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.setColour(GUIUniversals.colors.getColor(BDSP_COLOR_PURE_BLACK));
	g.fillAll();

	g.setColour(GUIUniversals.colors.getColor(BDSP_COLOR_DARK));
	g.fillRoundedRectangle(topBarArea, GUIUniversals.roundedRectangleCurve);



	g.fillRoundedRectangle(macroArea, GUIUniversals.roundedRectangleCurve);

}






void FlexFXAudioProcessorEditor::resized()
{

	if (!getBounds().isEmpty())
	{

		bdsp::Editor::resized();

		auto w = topLevelComp->mainArea.getWidth();
		auto h = topLevelComp->mainArea.getHeight();

		GUIUniversals.rectThicc = w / 200.0;


		border = GUIUniversals.rectThicc * 0.25;

		GUIUniversals.roundedRectangleCurve = w / 150.0;
		GUIUniversals.listBorderSize = GUIUniversals.rectThicc / 4;
		GUIUniversals.dividerSize = GUIUniversals.rectThicc / 6;
		GUIUniversals.dividerBorder = GUIUniversals.rectThicc;
		GUIUniversals.influenceHoverMenuHeight = h / 11.0 * GUIUniversals.systemScaleFactor;

		GUIUniversals.sliderPopupWidth = 2.5 * GUIUniversals.influenceHoverMenuHeight;

		topBarArea = topLevelComp->mainArea.getBounds().toFloat().withHeight(getHeight() * TopBarH).reduced(GUIUniversals.rectThicc);

		macroArea = topLevelComp->mainArea.getBounds().toFloat().getProportion(juce::Rectangle<float>(0, TopBarH, 1, MacroH)).reduced(GUIUniversals.rectThicc, 0);

		modArea = topLevelComp->mainArea.getBounds().toFloat().getProportion(juce::Rectangle<float>(0, TopBarH + MacroH, 1, ModH)).reduced(GUIUniversals.rectThicc, 0);



		FXArea = topLevelComp->mainArea.getBounds().toFloat().getProportion(juce::Rectangle<float>(0, TopBarH + ModH + MacroH, 1, FXH)).reduced(GUIUniversals.rectThicc);

		hintBarArea = topLevelComp->mainArea.getBounds().toFloat().withTop(FXArea.getBottom()).reduced(GUIUniversals.rectThicc, 0);

		titleBar->setBounds(bdsp::shrinkRectangleToInt(topBarArea));

		topLevelComp->preset->titleBar.setBounds(bdsp::shrinkRectangleToInt(titleBar->getLocalBounds().getProportion(juce::Rectangle<float>(0.25, 0.15, 0.5, 0.7))));
		topLevelComp->undoRedo->setBounds(bdsp::shrinkRectangleToInt(titleBar->getLocalBounds().getProportion(juce::Rectangle<float>(0.85, 0.2, 0.15, 0.6)).reduced(GUIUniversals.rectThicc)));
		topLevelComp->settingsButton.setBounds(bdsp::shrinkRectangleToInt(titleBar->getLocalBounds().getProportion(juce::Rectangle<float>(0.75, 0.2, 0.1, 0.6)).reduced(GUIUniversals.rectThicc)));

		//topLevelComp->logo->setBounds(bdsp::shrinkRectangleToInt(titleBar->getLocalBounds().getProportion(juce::Rectangle<float>(0, 0.15, 0.05, 0.7)).reduced(0, GUIUniversals.rectThicc)));

		title->setBounds(bdsp::shrinkRectangleToInt(titleBar->getLocalBounds().getProportion(juce::Rectangle<float>(0, 0.1, 0.25, 0.8)).reduced(GUIUniversals.rectThicc)));

		//================================================================================================================================================================================================
		modTabs->setBounds(bdsp::shrinkRectangleToInt(modArea));

		auto modBounds = modTabs->getUsableArea().withZeroOrigin();
		LFOs->setBounds(modBounds.reduced(GUIUniversals.rectThicc));
		EnvelopeFollowers->setBounds(modBounds.reduced(GUIUniversals.rectThicc));
		Macros->setBounds(bdsp::shrinkRectangleToInt(macroArea.reduced(GUIUniversals.rectThicc)));

		for (int i = 0; i < BDSP_NUMBER_OF_MACROS; ++i)
		{
			Macros->getMacro(i)->setBounds(bdsp::shrinkRectangleToInt(Macros->getLocalBounds().getProportion(juce::Rectangle<float>(i * 1.0 / BDSP_NUMBER_OF_MACROS, 0, 1.0 / BDSP_NUMBER_OF_MACROS, 1)).reduced(GUIUniversals.rectThicc, 0)));
		}


		//	routing->setBounds(bdsp::shrinkRectangleToInt(routingArea.reduced(GUIUniversals.rectThicc)));

			//================================================================================================================================================================================================

		fxSlotManager->setBounds(bdsp::shrinkRectangleToInt(FXArea));



		fxSectionSize = dynamic_cast<FXSlot*>(fxSlotManager->getComp(0))->getFXSectionBounds().withZeroOrigin();
		//================================================================================================================================================================================================		


		//================================================================================================================================================================================================
		GUIUniversals.hintBar->setBounds(bdsp::shrinkRectangleToInt(hintBarArea));

		//================================================================================================================================================================================================
		auto cornerS = 3 * GUIUniversals.rectThicc;

		corner.setBounds(juce::Rectangle<int>().leftTopRightBottom(getWidth() - cornerS, getHeight() - cornerS, getWidth(), getHeight()));

		//================================================================================================================================================================================================
		resizeFX();
	}
}












void FlexFXAudioProcessorEditor::setUniversals()
{

	auto font1 = juce::Font(juce::Typeface::createSystemTypefaceFor(Font_Data::BEBASREGULAR_TTF, Font_Data::BEBASREGULAR_TTFSize));
	GUIUniversals.Fonts.add(font1);

	auto font2 = juce::Font(juce::Typeface::createSystemTypefaceFor(Font_Data::BARLOWMEDIUM_TTF, Font_Data::BARLOWMEDIUM_TTFSize));
	GUIUniversals.Fonts.add(font2);


	auto font3 = juce::Font(juce::Typeface::createSystemTypefaceFor(Font_Data::CUNIA_TTF, Font_Data::CUNIA_TTFSize));
	GUIUniversals.Fonts.add(font3);


	auto font4 = juce::Font(juce::Typeface::createSystemTypefaceFor(Font_Data::BarlowLight_ttf, Font_Data::BarlowLight_ttfSize));
	GUIUniversals.Fonts.add(font4);




	//GUIUniversals.texture = audioProcessor.texture;



	GUIUniversals.colors.setColor(BDSP_COLOR_PURE_BLACK, juce::Colour(0, 0, 0));
	GUIUniversals.colors.setColor(BDSP_COLOR_BLACK, juce::Colour(26, 26, 26));
	GUIUniversals.colors.setColor(BDSP_COLOR_DARK, juce::Colour(38, 38, 38));
	GUIUniversals.colors.setColor(BDSP_COLOR_MID, juce::Colour(64, 64, 64));
	GUIUniversals.colors.setColor(BDSP_COLOR_LIGHT, juce::Colour(115, 115, 115));
	GUIUniversals.colors.setColor(BDSP_COLOR_WHITE, juce::Colour(217, 217, 217));
	GUIUniversals.colors.setColor(BDSP_COLOR_KNOB, juce::Colour(153, 153, 153));

	GUIUniversals.colors.setColor(BDSP_COLOR_COLOR, juce::Colour(215, 165, 243));


	//GUIUniversals.colors.setColor(FXTypeNames->operator[](0), GUIUniversals.colors.getColor(BDSP_COLOR_LIGHT)); // None
	//
	//GUIUniversals.colors.setColor(FXTypeNames->operator[](1), juce::Colour(190, 43, 27)); // Distortion
	//GUIUniversals.colors.setColor(FXTypeNames->operator[](2), juce::Colour(217, 91, 0)); // BitCrush


	//GUIUniversals.colors.setColor(FXTypeNames->operator[](3), juce::Colour(190, 146, 27)); // Filter

	//GUIUniversals.colors.setColor(FXTypeNames->operator[](4), juce::Colour(190, 27, 189)); // Pitch Shift

	//GUIUniversals.colors.setColor(FXTypeNames->operator[](5), juce::Colour(29, 181, 204)); // Chours
	//GUIUniversals.colors.setColor(FXTypeNames->operator[](6), juce::Colour(19, 178, 84)); // Flanger
	//GUIUniversals.colors.setColor(FXTypeNames->operator[](7), juce::Colour(71, 112, 235)); // Phaser


	//GUIUniversals.colors.setColor(FXTypeNames->operator[](8), juce::Colour(71, 112, 235)); // Panner	


	GUIUniversals.colors.setColor(FXTypeNames->operator[](0), GUIUniversals.colors.getColor(BDSP_COLOR_LIGHT)); // None

	GUIUniversals.colors.setColor(FXTypeNames->operator[](1), juce::Colour(190, 43, 27)); // Distortion
	GUIUniversals.colors.setColor(FXTypeNames->operator[](2), GUIUniversals.colors.getColor(FXTypeNames->operator[](1))); // BitCrush


	GUIUniversals.colors.setColor(FXTypeNames->operator[](3), juce::Colour(190, 146, 27)); // Filter
	GUIUniversals.colors.setColor(FXTypeNames->operator[](4), GUIUniversals.colors.getColor(FXTypeNames->operator[](3))); // EQ

	GUIUniversals.colors.setColor(FXTypeNames->operator[](5), juce::Colour(190, 27, 189)); // Pitch Shift
	GUIUniversals.colors.setColor(FXTypeNames->operator[](6), GUIUniversals.colors.getColor(FXTypeNames->operator[](5))); // RingMod

	GUIUniversals.colors.setColor(FXTypeNames->operator[](7), juce::Colour(29, 181, 204)); // Chours
	GUIUniversals.colors.setColor(FXTypeNames->operator[](8), GUIUniversals.colors.getColor(FXTypeNames->operator[](7))); // Flanger
	GUIUniversals.colors.setColor(FXTypeNames->operator[](9), GUIUniversals.colors.getColor(FXTypeNames->operator[](7))); // Phaser


	GUIUniversals.colors.setColor(FXTypeNames->operator[](10), juce::Colour(19, 178, 84)); // Panner
	GUIUniversals.colors.setColor(FXTypeNames->operator[](11), GUIUniversals.colors.getColor(FXTypeNames->operator[](10))); // Noise




}


bdsp::RangedCircleSlider* FlexFXAudioProcessorEditor::createRangedCircleSlider(const juce::String& name, juce::Component* parentSection, juce::OwnedArray<bdsp::RangedCircleSlider>& arrayToAddTo, const bdsp::NamedColorsIdentifier& color, const juce::String& label)
{
	arrayToAddTo.add(new bdsp::RangedCircleSlider(&GUIUniversals, name));
	bdsp::CircleSlider& current = *arrayToAddTo.getLast();





	parentSection->addAndMakeVisible(arrayToAddTo.getLast());
	current.attach(*audioProcessor.parameters.get(), current.getComponentID());

	current.setSliderColor(color, BDSP_COLOR_WHITE);
	current.label.setTitle(label);
	current.initText();

	return arrayToAddTo.getLast();
}


void FlexFXAudioProcessorEditor::initDistortions()
{
	auto color = bdsp::NamedColorsIdentifier(FXTypeNames->operator[](FlexFXAudioProcessor::Distortion));
	for (int i = 0; i < numFXSlots; ++i)
	{
		distortionSections.add(new bdsp::Component(&GUIUniversals));
		auto distSection = distortionSections[i];
		auto name = juce::String(i + 1);

		GUIUniversals.hintBar->addTextToRecolor({ "Distortion " + name, color });
		GUIUniversals.hintBar->addTextToRecolor({ "Distortion " + name + "'s", color });

		//================================================================================================================================================================================================


		auto preName = "Distortion " + name + " Pre Gain";
		distortionPreGainSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, preName));
		bdsp::CircleSlider& currentPre = *distortionPreGainSliders[i];





		distSection->addAndMakeVisible(distortionPreGainSliders[i]);
		currentPre.attach(*audioProcessor.parameters.get(), currentPre.getComponentID());

		currentPre.setSliderColor(color, BDSP_COLOR_WHITE);
		currentPre.label.setTitle(juce::String("Pre-Gain"));
		currentPre.initText();

		currentPre.setHintBarText("Gain of input into Distortion " + name);
		currentPre.addCatchValues({ 1 }, false);

		//================================================================================================================================================================================================


		auto amountName = "Distortion " + name + " Amount";
		distortionAmountSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, amountName));
		bdsp::CircleSlider& currentAmount = *distortionAmountSliders[i];





		distSection->addAndMakeVisible(distortionAmountSliders[i]);
		currentAmount.attach(*audioProcessor.parameters.get(), currentAmount.getComponentID());

		currentAmount.setSliderColor(color, BDSP_COLOR_WHITE);
		currentAmount.label.setTitle(juce::String("Amt"));
		currentAmount.initText();

		currentAmount.setHintBarText("Intensity of Distortion " + name);


		//================================================================================================================================================================================================


		auto mixName = "FX " + name + " Mix";
		distortionMixSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, mixName));
		bdsp::CircleSlider& currentMix = *distortionMixSliders[i];





		distSection->addAndMakeVisible(distortionMixSliders[i]);
		currentMix.attach(*audioProcessor.parameters.get(), currentMix.getComponentID());

		currentMix.setSliderColor(color, BDSP_COLOR_WHITE);
		currentMix.label.setTitle(juce::String("Mix"));
		currentMix.initText();

		currentMix.setHintBarText("Dry/Wet mix of Distortion " + name);




		//================================================================================================================================================================================================
		distortionAGCSections.add(new bdsp::Component(&GUIUniversals));
		distSection->addAndMakeVisible(distortionAGCSections[i]);

		distortionAGCButtons.add(new bdsp::ToggleSlider(&GUIUniversals, ""));
		auto currentAGC = distortionAGCButtons[i];


		currentAGC->setFontIndex(2);
		currentAGC->setColors(color, bdsp::NamedColorsIdentifier());

		currentAGC->attach(*audioProcessor.parameters.get(), "Distortion" + name + "GainCompensationID");

		distortionAGCSections[i]->addAndMakeVisible(distortionAGCButtons[i]);

		currentAGC->onStateChange = [=]()
		{
			currentAGC->setHintBarText(juce::String(currentAGC->getToggleState() ? "Disables " : "Enables ") + "automatic gain compensation on Distortion " + name + "'s output");
		};

		distortionAGCLabels.add(new bdsp::MousePassthrough<bdsp::Label>(currentAGC, &GUIUniversals));
		auto currentAGCLabel = distortionAGCLabels[i];
		distortionAGCSections[i]->addAndMakeVisible(distortionAGCLabels[i]);

		currentAGCLabel->setText("Auto", juce::sendNotification);
		//		//currentAGCLabel->setLookAndFeel(&GUIUniversals.MasterCircleSliderLNF);
		currentAGCLabel->setJustificationType(juce::Justification::centred);
		currentAGCLabel->setColour(juce::Label::textColourId, GUIUniversals.colors.getColor(color));
		//================================================================================================================================================================================================

		distortionVisualizers.add(new bdsp::DistortionVisualizer(&GUIUniversals, &audioProcessor.lookups, &currentAmount, &distortionMixSliders[i]->slider, currentAGC));
		auto currentVis = distortionVisualizers[i];
		distSection->addAndMakeVisible(currentVis);
		currentVis->setColor(color);
		currentVis->setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_BLACK);

		//DistortionVisualizer->isScaled = true;


		//================================================================================================================================================================================================

		auto DistortionTypeParam = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.parameters->getParameter("Distortion" + name + "TypeID"));

		juce::Array<juce::Path> DistortionTypeShapes;
		for (int i = 0; i < DistortionTypeParam->choices.size(); ++i)
		{
			auto p = GUIUniversals.commonPaths.distortionPaths.getPath(audioProcessor.lookups.distortionLookups->stringToDistortionType(DistortionTypeParam->choices[i]));
			juce::PathStrokeType(p.getBounds().getWidth() * 0.1).createStrokedPath(p, p);
			DistortionTypeShapes.add(p);
		}



		distortionTypeSelectors.add(new bdsp::IncrementalComboBox<bdsp::AdvancedComboBox>(DistortionTypeParam, &GUIUniversals, DistortionTypeShapes));
		auto currentTypeSelector = distortionTypeSelectors[i];

		currentTypeSelector->setIconBorderSize(juce::BorderSize<float>(0.1, 0, 0.1, 0.1));
		currentTypeSelector->onChange = [=](int i)
		{
			//DistortionVisualizer->setType(static_cast<bdsp::dsp::DistortionLookupTables>(1 << i));
			auto text = DistortionTypeParam->getCurrentChoiceName();
			auto type = audioProcessor.lookups.distortionLookups->stringToDistortionType(text);

			currentVis->setType(type);


			currentTypeSelector->setHintBarText("Transfer function of Distortion " + name);
			//if (currentTypeSelector->getButton() || currentTypeSelector->left->isMouseOver() || currentTypeSelector->right->isMouseOver())
			//{
			//	GUIUniversals.hintBar->setText(currentTypeSelector->getHintText());
			//}
		};
		currentTypeSelector->setMenuWidth(1);
		currentTypeSelector->setFontIndex(0);
		currentTypeSelector->setJustification(juce::Justification::centredLeft);

		currentTypeSelector->setRowsAndCols(DistortionTypeParam->choices.size(), 1);
		currentTypeSelector->attach(DistortionTypeParam);

		currentTypeSelector->onChange(currentTypeSelector->getIndex());
		currentTypeSelector->setColorSchemeMinimal(bdsp::NamedColorsIdentifier(), BDSP_COLOR_PURE_BLACK, BDSP_COLOR_DARK, color, BDSP_COLOR_DARK);

		//DistortionType->setMenuWidth(2.75f);

		distSection->addAndMakeVisible(currentTypeSelector);
		//DistortionType->getListuniversals()->setLossOfFocusClosesWindow(true, DistortionSection->getTitleComponent());

		//================================================================================================================================================================================================






	}
}

void FlexFXAudioProcessorEditor::resizeDistortions()
{
	auto topH = 0.325;
	auto visH = 0.5;
	auto selectorH = 0.175;
	auto bounds = fxSectionSize.toFloat();
	for (int i = 0; i < numFXSlots; ++i)
	{
		distortionSections[i]->setBounds(fxSectionSize);
		distortionPreGainSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, 0, 0.25, topH)).reduced(GUIUniversals.rectThicc)));
		distortionAmountSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.25, 0, 0.25, topH)).reduced(GUIUniversals.rectThicc)));

		distortionAGCSections[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.5, 0, 0.25, topH)).reduced(GUIUniversals.rectThicc)));
		distortionAGCButtons[i]->setBounds(distortionPreGainSliders[i]->slider.getKnobBounds(true));
		distortionAGCLabels[i]->setBounds(distortionPreGainSliders[i]->slider.label.getBounds());

		distortionMixSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.75, 0, 0.25, topH)).reduced(GUIUniversals.rectThicc)));


		distortionTypeSelectors[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, topH, 1, selectorH)).reduced(GUIUniversals.rectThicc)));
		distortionVisualizers[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, topH + selectorH, 1, visH)).reduced(GUIUniversals.rectThicc)));

	}
}

void FlexFXAudioProcessorEditor::swapDistortions(int idx1, int idx2)
{
	distortionPreGainSliders[idx1]->swap(distortionPreGainSliders[idx2]);
	distortionAmountSliders[idx1]->swap(distortionAmountSliders[idx2]);
	distortionMixSliders[idx1]->swap(distortionMixSliders[idx2]);

	distortionAGCButtons[idx1]->swapToggleStates(distortionAGCButtons[idx2], juce::sendNotification);

	int type2 = distortionTypeSelectors[idx2]->getIndex();
	distortionTypeSelectors[idx2]->selectItem(distortionTypeSelectors[idx1]->getIndex());
	distortionTypeSelectors[idx1]->selectItem(type2);
}

void FlexFXAudioProcessorEditor::initBitcrushes()
{
	auto color = bdsp::NamedColorsIdentifier(FXTypeNames->operator[](FlexFXAudioProcessor::BitCrush));
	for (int i = 0; i < numFXSlots; ++i)
	{
		bitcrushSections.add(new bdsp::Component(&GUIUniversals));
		auto bitcrushSection = bitcrushSections[i];
		auto name = juce::String(i + 1);

		GUIUniversals.hintBar->addTextToRecolor({ "Bit Crush " + name, color });
		GUIUniversals.hintBar->addTextToRecolor({ "Bit Crush " + name + "'s", color });

		//================================================================================================================================================================================================

		createRangedCircleSlider("Bit Crush" + name + " Depth", bitcrushSection, bitcrushDepthSliders, color, "Bit Depth");

		bitcrushDepthSliders.getLast()->setHintBarText("Amplitude resolution of Bit Crush " + name);

		//================================================================================================================================================================================================


		createRangedCircleSlider("Bit Crush" + name + " Rate", bitcrushSection, bitcrushRateSliders, color, "Rate");

		bitcrushRateSliders.getLast()->setHintBarText("Sampling rate of Bit Crush " + name);

		//================================================================================================================================================================================================

		createRangedCircleSlider("FX" + name + " Mix", bitcrushSection, bitcrushMixSliders, color, "Mix");

		bitcrushMixSliders.getLast()->setHintBarText("Dry/Wet mix of Bit Crush " + name);



		//================================================================================================================================================================================================

		bitcrushVisualizers.add(new bdsp::BitCrushVisualizer(&GUIUniversals, &bitcrushDepthSliders.getLast()->slider, &bitcrushRateSliders.getLast()->slider, &bitcrushMixSliders.getLast()->slider));
		auto currentVis = bitcrushVisualizers[i];
		bitcrushSection->addAndMakeVisible(currentVis);
		currentVis->setColor(color);
		currentVis->setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_BLACK);
		currentVis->getVis()->setScaling(0.9, 0.9);





	}
}

void FlexFXAudioProcessorEditor::resizeBitcrushes()
{
	auto topH = 0.35;
	auto visH = 0.65;
	auto bounds = fxSectionSize.toFloat();
	for (int i = 0; i < numFXSlots; ++i)
	{
		bitcrushSections[i]->setBounds(fxSectionSize);
		bitcrushDepthSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, 0, 1.0 / 3.0, topH)).reduced(GUIUniversals.rectThicc)));
		bitcrushRateSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(1.0 / 3.0, 0, 1.0 / 3.0, topH)).reduced(GUIUniversals.rectThicc)));
		bitcrushMixSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(2.0 / 3.0, 0, 1.0 / 3.0, topH)).reduced(GUIUniversals.rectThicc)));

		bitcrushVisualizers[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, topH, 1, visH)).reduced(GUIUniversals.rectThicc)));
	}
}

void FlexFXAudioProcessorEditor::swapBitcrushes(int idx1, int idx2)
{
	bitcrushDepthSliders[idx1]->swap(bitcrushDepthSliders[idx2]);
	bitcrushRateSliders[idx1]->swap(bitcrushRateSliders[idx2]);
	bitcrushMixSliders[idx1]->swap(bitcrushMixSliders[idx2]);
}


//================================================================================================================================================================================================


void FlexFXAudioProcessorEditor::initFilters()
{
	auto color = bdsp::NamedColorsIdentifier(FXTypeNames->operator[](FlexFXAudioProcessor::Filter));
	for (int i = 0; i < numFXSlots; ++i)
	{
		filterSections.add(new bdsp::Component(&GUIUniversals));
		auto filterSection = filterSections[i];
		auto name = juce::String(i + 1);

		GUIUniversals.hintBar->addTextToRecolor({ "Filter " + name, color });
		GUIUniversals.hintBar->addTextToRecolor({ "Filter " + name + "'s", color });

		//================================================================================================================================================================================================


		auto typeName = "Filter " + name + " Type";
		filterTypeSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, typeName));
		bdsp::CircleSlider& currentType = *filterTypeSliders[i];





		filterSection->addAndMakeVisible(filterTypeSliders[i]);
		currentType.attach(*audioProcessor.parameters.get(), currentType.getComponentID());

		currentType.setSliderColor(color, BDSP_COLOR_WHITE);
		currentType.label.setTitle(juce::String("Type"));
		currentType.initText();

		currentType.setHintBarText("Type of Filter " + name);

		//================================================================================================================================================================================================
		auto freqName = "Filter " + name + " Frequency";
		filterFreqSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, freqName));
		bdsp::CircleSlider& currentFreq = *filterFreqSliders[i];





		filterSection->addAndMakeVisible(filterFreqSliders[i]);
		currentFreq.attach(*audioProcessor.parameters.get(), currentFreq.getComponentID());

		currentFreq.setSliderColor(color, BDSP_COLOR_WHITE);
		currentFreq.label.setTitle(juce::String("Freq"));
		currentFreq.initText();

		currentFreq.setHintBarText("Cutoff frequency of Filter " + name);

		//================================================================================================================================================================================================
		auto qName = "Filter " + name + " Resonance";
		filterQSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, qName));
		bdsp::CircleSlider& currentQ = *filterQSliders[i];





		filterSection->addAndMakeVisible(filterQSliders[i]);
		currentQ.attach(*audioProcessor.parameters.get(), currentQ.getComponentID());

		currentQ.setSliderColor(color, BDSP_COLOR_WHITE);
		currentQ.label.setTitle(juce::String("Res"));
		currentQ.initText();

		currentQ.setHintBarText("Resonance of Filter " + name);

		//================================================================================================================================================================================================


		auto mixName = "FX " + name + " Mix";
		filterMixSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, mixName));
		bdsp::CircleSlider& currentMix = *filterMixSliders[i];





		filterSection->addAndMakeVisible(filterMixSliders[i]);
		currentMix.attach(*audioProcessor.parameters.get(), currentMix.getComponentID());

		currentMix.setSliderColor(color, BDSP_COLOR_WHITE);
		currentMix.label.setTitle(juce::String("Mix"));
		currentMix.initText();

		currentMix.setHintBarText("Dry/Wet mix of Filter " + name);




		//================================================================================================================================================================================================

		filterVisualizers.add(new bdsp::FilterVisualizer(&GUIUniversals, dynamic_cast<FlexFXAudioProcessor*>(&audioProcessor)->filters[i], &currentType, &currentFreq, &currentQ, &filterMixSliders[i]->slider));
		auto currentVis = filterVisualizers[i];
		filterSection->addAndMakeVisible(currentVis);
		currentVis->setColor(color);
		currentVis->setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_BLACK);

	}
}

void FlexFXAudioProcessorEditor::resizeFilters()
{
	auto topH = 0.35;
	auto visH = 0.65;
	auto bounds = fxSectionSize.toFloat();
	for (int i = 0; i < numFXSlots; ++i)
	{
		filterSections[i]->setBounds(fxSectionSize);
		filterTypeSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, 0, 0.25, topH)).reduced(GUIUniversals.rectThicc)));
		filterFreqSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.25, 0, 0.25, topH)).reduced(GUIUniversals.rectThicc)));
		filterQSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.5, 0, 0.25, topH)).reduced(GUIUniversals.rectThicc)));
		filterMixSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.75, 0, 0.25, topH)).reduced(GUIUniversals.rectThicc)));


		filterVisualizers[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, topH, 1, visH)).reduced(GUIUniversals.rectThicc)));
	}
}

void FlexFXAudioProcessorEditor::swapFilters(int idx1, int idx2)
{
	filterTypeSliders[idx1]->swap(filterTypeSliders[idx2]);
	filterFreqSliders[idx1]->swap(filterFreqSliders[idx2]);
	filterQSliders[idx1]->swap(filterQSliders[idx2]);
	filterMixSliders[idx1]->swap(filterMixSliders[idx2]);
}


//================================================================================================================================================================================================

void FlexFXAudioProcessorEditor::initEQs()
{

	auto color = bdsp::NamedColorsIdentifier(FXTypeNames->operator[](FlexFXAudioProcessor::EQ));
	for (int i = 0; i < numFXSlots; ++i)
	{
		EQSections.add(new bdsp::Component(&GUIUniversals));
		auto EQSection = EQSections[i];
		auto name = juce::String(i + 1);

		GUIUniversals.hintBar->addTextToRecolor({ "EQ " + name, color });
		GUIUniversals.hintBar->addTextToRecolor({ "EQ " + name + "'s", color });

		juce::StringArray tabs;
		tabs.add("Low");

		for (int j = 1; j < EQNumBands - 1; ++j)
		{
			tabs.add(juce::String(j));
		}
		tabs.add("High");

		EQTabs.add(new bdsp::TabsComponent(&GUIUniversals, tabs));
		EQTabs.getLast()->setVertical(false);
		EQTabs.getLast()->setTabRatio(0.25);
		EQTabs.getLast()->setColors(BDSP_COLOR_DARK, color);
		EQSection->addAndMakeVisible(EQTabs.getLast());


		auto mix = createRangedCircleSlider("FX " + name + " Mix", EQSection, EQMixSliders, color, "Mix");

		mix->slider.setHintBarText("Dry/Wet mix of EQ " + name);


		auto globalGain = createRangedCircleSlider("EQ " + name + "Global Gain", EQSection, EQGlobalGainSliders, color, "Global");
		globalGain->slider.setHintBarText("Adjust all band's gain values at once");
		globalGain->slider.addCatchValues({ 0 }, false);
		globalGain->slider.setType(bdsp::BaseSlider::CenterZero);

		EQVisualizers.add(new bdsp::EQVisualizer(&GUIUniversals, dynamic_cast<FlexFXAudioProcessor*>(&audioProcessor)->EQs[i], 100));

		EQVisualizers.getLast()->setColor(color);

		EQVisualizers.getLast()->getVis()->setGlobalSliders(&globalGain->slider, &mix->slider);

		EQVisualizers[i]->getVis()->onHandleSelected = [=](int idx)
		{
			EQTabs[i]->selectTab(idx);
		};
		EQTabs[i]->onTabChange = [=](int idx)
		{
			EQVisualizers[i]->selectHandle(idx);
		};
		EQSection->addAndMakeVisible(EQVisualizers.getLast());




		//================================================================================================================================================================================================
		for (int j = 0; j < EQNumBands; ++j)
		{
			auto bandName = "EQ " + name + " Band " + juce::String(j);


			auto gain = createRangedCircleSlider(bandName + " Gain", EQTabs[i]->getPage(j), EQGainSliders[i], color, "Gain");

			gain->slider.setHintBarText("Gain of " + bandName);


			auto freq = createRangedCircleSlider(bandName + " Frequency", EQTabs[i]->getPage(j), EQFreqSliders[i], color, "Freq");

			freq->slider.setHintBarText("Frequency of " + bandName);

			auto BW = createRangedCircleSlider(bandName + " Q", EQTabs[i]->getPage(j), EQBWSLiders[i], color, "Q");

			BW->slider.setHintBarText("Q Factor of " + bandName);


			EQVisualizers[i]->getVis()->setSliders(j, &freq->slider, &BW->slider, &gain->slider);
		}






	}
}

void FlexFXAudioProcessorEditor::resizeEQs()
{
	auto row1 = 0.35;
	auto tabH = 0.35;
	auto visH = 0.3;
	auto bounds = fxSectionSize.toFloat();
	for (int i = 0; i < numFXSlots; ++i)
	{
		EQSections[i]->setBounds(fxSectionSize);


		EQGlobalGainSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, 0, 0.5, row1)).reduced(GUIUniversals.rectThicc)));
		EQMixSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.5, 0, 0.5, row1)).reduced(GUIUniversals.rectThicc)));
		EQTabs[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, row1, 1, tabH)).reduced(GUIUniversals.rectThicc)));

		auto tabBounds = EQTabs[i]->getPage(0)->getLocalBounds();
		for (int j = 0; j < EQNumBands; ++j)
		{
			EQFreqSliders[i][j]->setBounds(bdsp::shrinkRectangleToInt(tabBounds.getProportion(juce::Rectangle<float>(0, 0, 1.0 / 3.0, 1)).reduced(GUIUniversals.rectThicc)));
			EQBWSLiders[i][j]->setBounds(bdsp::shrinkRectangleToInt(tabBounds.getProportion(juce::Rectangle<float>(1.0 / 3.0, 0, 1.0 / 3.0, 1)).reduced(GUIUniversals.rectThicc)));
			EQGainSliders[i][j]->setBounds(bdsp::shrinkRectangleToInt(tabBounds.getProportion(juce::Rectangle<float>(2.0 / 3.0, 0, 1.0 / 3.0, 1)).reduced(GUIUniversals.rectThicc)));
		}

		EQVisualizers[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, row1 + tabH, 1, visH)).reduced(GUIUniversals.rectThicc)));




	}
}

void FlexFXAudioProcessorEditor::swapEQs(int idx1, int idx2)
{
	for (int j = 0; j < EQNumBands; ++j)
	{
		EQFreqSliders[idx1][j]->swap(EQFreqSliders[idx2][j]);
		EQBWSLiders[idx1][j]->swap(EQBWSLiders[idx2][j]);
		EQGainSliders[idx1][j]->swap(EQGainSliders[idx2][j]);
	}
	EQMixSliders[idx1]->swap(EQMixSliders[idx2]);



}





//================================================================================================================================================================================================



void FlexFXAudioProcessorEditor::initPitchShifts()
{
	auto color = bdsp::NamedColorsIdentifier(FXTypeNames->operator[](FlexFXAudioProcessor::PitchShift));
	for (int i = 0; i < numFXSlots; ++i)
	{
		pitchShiftSections.add(new bdsp::Component(&GUIUniversals));
		auto pitchShiftSection = pitchShiftSections[i];
		auto name = juce::String(i + 1);

		GUIUniversals.hintBar->addTextToRecolor({ "Pitch Shift " + name, color });
		GUIUniversals.hintBar->addTextToRecolor({ "Pitch Shift " + name + "'s", color });

		//================================================================================================================================================================================================


		auto leftname = "Pitch Shift " + name + " Left";
		pitchShiftLeftSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, leftname));
		bdsp::CircleSlider& currentLeft = *pitchShiftLeftSliders[i];





		pitchShiftSection->addAndMakeVisible(pitchShiftLeftSliders[i]);
		currentLeft.attach(*audioProcessor.parameters.get(), currentLeft.getComponentID());

		currentLeft.setSliderColor(color, BDSP_COLOR_WHITE);
		currentLeft.label.setTitle(juce::String("L"));
		currentLeft.initText();

		currentLeft.setHintBarText("Shift amount of Pitch Shift " + name + "'s left channel (in semitones)");
		currentLeft.addCatchValues({ -12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12 }, false);


		//================================================================================================================================================================================================
		auto linkDummyName = "Pitch Shift " + name + " Link Dummy";
		pitchShiftLinkDummySliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, linkDummyName));
		bdsp::CircleSlider& currentDummy = *pitchShiftLinkDummySliders[i];





		pitchShiftSection->addChildComponent(pitchShiftLinkDummySliders[i]);
		currentDummy.attach(*audioProcessor.parameters.get(), currentLeft.getComponentID());

		currentDummy.setSliderColor(color, BDSP_COLOR_WHITE);
		currentDummy.label.setTitle(juce::String("R"));
		currentDummy.initText();


		currentDummy.setHintBarText("Shift amount of Pitch Shift " + name + "'s right channel (in semitones)");

		currentDummy.addCatchValues({ -12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12 }, false);

		pitchShiftLeftSliders[i]->addHoverPartner(pitchShiftLinkDummySliders[i]);





		auto prevLeftShow = pitchShiftLeftSliders[i]->hoverMenu->onShow;
		pitchShiftLeftSliders[i]->hoverMenu->onShow = [=]()
		{
			prevLeftShow();
			if (pitchShiftLinkButtons[i]->getToggleState())
			{
				pitchShiftLinkDummySliders[i]->hoverMenu->hide();
				pitchShiftLinkDummySliders[i]->hoverMenu->resizeMenu();
				pitchShiftLeftSliders[i]->hoverMenu->setBounds(pitchShiftLeftSliders[i]->hoverMenu->getBounds().withCentre(pitchShiftLinkDummySliders[i]->hoverMenu->getBounds().getCentre()));
			}
		};

		auto prevDummyShow = pitchShiftLinkDummySliders[i]->hoverMenu->onShow;
		pitchShiftLinkDummySliders[i]->hoverMenu->onShow = [=]()
		{
			prevDummyShow();

			pitchShiftLeftSliders[i]->hoverMenu->hide();
		};

		for (int j = 0; j < BDSP_NUMBER_OF_LINKABLE_CONTROLS; ++j)
		{
			auto prevLeftFunc = pitchShiftLeftSliders[i]->influences[j]->getHoverFunc();

			std::function<void(bool)> leftFunc = [=](bool state)
			{
				prevLeftFunc(state);
				pitchShiftLinkDummySliders[i]->displays[j]->setVisible(state);
			};
			pitchShiftLeftSliders[i]->influences[j]->setHoverFunc(leftFunc);

			//================================================================================================================================================================================================

			auto prevDummyFunc = pitchShiftLinkDummySliders[i]->influences[j]->getHoverFunc();

			std::function<void(bool)> dummyFunc = [=](bool state)
			{
				prevDummyFunc(state);
				pitchShiftLeftSliders[i]->displays[j]->setVisible(state);
			};
			pitchShiftLinkDummySliders[i]->influences[j]->setHoverFunc(dummyFunc);
		}
		//================================================================================================================================================================================================

		auto rightName = "Pitch Shift " + name + " Right";
		pitchShiftRightSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, rightName));
		bdsp::CircleSlider& currentRight = *pitchShiftRightSliders[i];





		pitchShiftSection->addAndMakeVisible(pitchShiftRightSliders[i]);
		currentRight.attach(*audioProcessor.parameters.get(), currentRight.getComponentID());

		currentRight.setSliderColor(color, BDSP_COLOR_WHITE);
		currentRight.label.setTitle(juce::String("R"));
		currentRight.initText();

		currentRight.setHintBarText("Shift amount of Pitch Shift " + name + "'s Right channel (in semitones)");
		currentRight.addCatchValues({ -12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12 }, false);



		//================================================================================================================================================================================================


		auto mixName = "FX " + name + " Mix";
		pitchShiftMixSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, mixName));
		bdsp::CircleSlider& currentMix = *pitchShiftMixSliders[i];





		pitchShiftSection->addAndMakeVisible(pitchShiftMixSliders[i]);
		currentMix.attach(*audioProcessor.parameters.get(), currentMix.getComponentID());

		currentMix.setSliderColor(color, BDSP_COLOR_WHITE);
		currentMix.label.setTitle(juce::String("Mix"));
		currentMix.initText();

		currentMix.setHintBarText("Dry/Wet mix of Pitch Shift " + name);





		//================================================================================================================================================================================================

		pitchShiftVisualizers.add(new bdsp::PitchShifterVisualizer(&GUIUniversals, &currentLeft, &currentRight, &pitchShiftMixSliders[i]->slider));
		auto currentVis = pitchShiftVisualizers[i];
		pitchShiftSection->addAndMakeVisible(currentVis);
		currentVis->setColor(color, BDSP_COLOR_DARK);
		currentVis->setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_BLACK);

		currentVis->setBarWidth(0.01f, 0.0001f);
		currentVis->setDotSize(0.05f);

		//================================================================================================================================================================================================

		pitchShiftLinkButtons.add(new bdsp::PathButton(&GUIUniversals, true, false));

		pitchShiftSection->addAndMakeVisible(pitchShiftLinkButtons[i]);

		pitchShiftLinkButtons[i]->setPath(GUIUniversals.commonPaths.linkSymbol);
		pitchShiftLinkButtons[i]->setColor(BDSP_COLOR_KNOB);




		pitchShiftLinkButtons[i]->onStateChange = [=]()
		{
			auto on = pitchShiftLinkButtons[i]->getToggleState();

			pitchShiftRightSliders[i]->setVisible(!on);
			pitchShiftLinkDummySliders[i]->setVisible(on);

			pitchShiftVisualizers[i]->linkAmounts(on);

			pitchShiftLinkButtons[i]->setHintBarText(juce::String(on ? "Unlinks" : "Links") + " Pitch Shift " + name + "'s left and right channel's");
		};

		pitchShiftLinkButtons[i]->attach(*audioProcessor.parameters.get(), "PitchShift" + name + "LinkID");
	}
}

void FlexFXAudioProcessorEditor::resizePitchShifts()
{
	auto row1 = 0.35;
	auto buttonH = 0.15;
	auto visH = 0.65;
	auto bounds = fxSectionSize.toFloat();
	for (int i = 0; i < numFXSlots; ++i)
	{
		pitchShiftSections[i]->setBounds(fxSectionSize);
		pitchShiftLeftSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, 0, 0.25, row1)).reduced(GUIUniversals.rectThicc)));
		pitchShiftRightSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.5, 0, 0.25, row1)).reduced(GUIUniversals.rectThicc)));
		pitchShiftMixSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.75, 0, 0.25, row1)).reduced(GUIUniversals.rectThicc)));

		auto linkButtonBounds = bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.25, (row1 - buttonH) / 2, 0.25, buttonH)).reduced(GUIUniversals.rectThicc));
		pitchShiftLinkButtons[i]->setBounds(linkButtonBounds.withCentre({ linkButtonBounds.getCentreX() , pitchShiftLeftSliders[i]->getY() + pitchShiftLeftSliders[i]->slider.getKnobBounds(true).getCentreY() }));
		//pitchShiftLinkButtons[i]->setBounds(pitchShiftLeftSliders[i]->slider.getKnobBounds(true).withCentreX((pitchShiftLeftSliders[i]->getRight()+pitchShiftRightSliders[i]->getX()) / 2));

		pitchShiftLinkDummySliders[i]->setBounds(pitchShiftRightSliders[i]->getBounds());

		//pitchShiftLinkButtons[i]->setBounds(pitchShiftLeftSliders[i]->getBounds().withCentreX((pitchShiftLeftSliders[i]->getRight() + pitchShiftRightSliders[i]->getX()) / 2.0));

		pitchShiftVisualizers[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, 1 - visH, 1, visH)).reduced(GUIUniversals.rectThicc)));
	}
}

void FlexFXAudioProcessorEditor::swapPitchShifts(int idx1, int idx2)
{

	pitchShiftLeftSliders[idx1]->swap(pitchShiftLeftSliders[idx2]);
	pitchShiftRightSliders[idx1]->swap(pitchShiftRightSliders[idx2]);
	pitchShiftLinkDummySliders[idx1]->swap(pitchShiftLinkDummySliders[idx2]);
	pitchShiftMixSliders[idx1]->swap(pitchShiftMixSliders[idx2]);


	pitchShiftLinkButtons[idx1]->swapToggleStates(pitchShiftLinkButtons[idx2], juce::sendNotification);
}



//================================================================================================================================================================================================



void FlexFXAudioProcessorEditor::initRingMods()
{
	auto color = bdsp::NamedColorsIdentifier(FXTypeNames->operator[](FlexFXAudioProcessor::RingMod));
	for (int i = 0; i < numFXSlots; ++i)
	{
		ringModSections.add(new bdsp::Component(&GUIUniversals));
		auto ringModSection = ringModSections[i];
		auto name = juce::String(i + 1);

		GUIUniversals.hintBar->addTextToRecolor({ "Ring Mod " + name, color });
		GUIUniversals.hintBar->addTextToRecolor({ "Ring Mod " + name + "'s", color });

		//================================================================================================================================================================================================


		auto shape = createRangedCircleSlider("Ring Mod " + name + " Shape", ringModSection, ringModShapeSliders, color, "Shape");

		shape->slider.setHintBarText("Shape of Ring Mod " + name);

		shape->slider.addCatchValues({ 0.5 });

		auto skew = createRangedCircleSlider("Ring Mod " + name + " Skew", ringModSection, ringModSkewSliders, color, "Skew");

		skew->slider.setHintBarText("Skew of Ring Mod " + name);
		skew->slider.addCatchValues({ 0.5 });

		auto freq = createRangedCircleSlider("Ring Mod " + name + " Frequency", ringModSection, ringModFreqSliders, color, "Freq");

		freq->slider.setHintBarText("Frequency of Ring Mod " + name);


		auto mix = createRangedCircleSlider("FX " + name + " Mix", ringModSection, ringModMixSliders, color, "Mix");

		mix->slider.setHintBarText("Dry/Wet mix of Ring Mod " + name);


		auto sourceParam = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.parameters->getParameter("RingMod" + name + "SourceID"));
		ringModSourceCombos.add(new bdsp::ComboBox(sourceParam, &GUIUniversals));
		ringModSection->addAndMakeVisible(ringModSourceCombos.getLast());

		ringModSourceCombos.getLast()->onChange = [=](int idx)
		{
			ringModShapeSliders[i]->setEnabled(idx == 0);
			ringModSkewSliders[i]->setEnabled(idx == 0);
			ringModFreqSliders[i]->setEnabled(idx == 0);

			ringModVisualizers[i]->getVis()->setSource(bdsp::dsp::RingModulation<float>::RingModSource(idx + 1));
		};



		//================================================================================================================================================================================================
		ringModVisualizers.add(new bdsp::RingModVisualizer(&GUIUniversals, &audioProcessor.lookups, sampleRate));
		auto currentVis = ringModVisualizers[i]->getVis();
		ringModSection->addAndMakeVisible(ringModVisualizers[i]);
		currentVis->setColor(color, BDSP_COLOR_DARK);

		currentVis->shapeParam = shape->getControlParamter();
		currentVis->skewParam = skew->getControlParamter();
		currentVis->freqParam = freq->getControlParamter();
		currentVis->mixParam = mix->getControlParamter();

		ringModSourceCombos.getLast()->onChange(ringModSourceCombos.getLast()->getIndex());
	}
}

void FlexFXAudioProcessorEditor::resizeRingMods()
{
	auto row1 = 0.35;
	auto row2 = 0.35;
	auto visH = 0.3;
	auto bounds = fxSectionSize.toFloat();
	for (int i = 0; i < numFXSlots; ++i)
	{
		ringModSections[i]->setBounds(fxSectionSize);
		ringModSourceCombos[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, 0, 2.0 / 3.0, row1)).reduced(GUIUniversals.rectThicc)));
		ringModMixSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(2.0 / 3.0, 0, 1.0 / 3.0, row1)).reduced(GUIUniversals.rectThicc)));

		ringModShapeSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, row1, 1.0 / 3.0, row2)).reduced(GUIUniversals.rectThicc)));
		ringModSkewSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(1.0 / 3.0, row1, 1.0 / 3.0, row2)).reduced(GUIUniversals.rectThicc)));
		ringModFreqSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(2.0 / 3.0, row1, 1.0 / 3.0, row2)).reduced(GUIUniversals.rectThicc)));

		ringModVisualizers[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, row1 + row2, 1, visH)).reduced(GUIUniversals.rectThicc)));
	}
}

void FlexFXAudioProcessorEditor::swapRingMods(int idx1, int idx2)
{
	ringModShapeSliders[idx1]->swap(ringModMixSliders[idx2]);
	ringModSkewSliders[idx1]->swap(ringModMixSliders[idx2]);
	ringModFreqSliders[idx1]->swap(ringModMixSliders[idx2]);
	ringModMixSliders[idx1]->swap(ringModMixSliders[idx2]);

	int type2 = ringModSourceCombos[idx2]->getIndex();
	ringModSourceCombos[idx2]->selectItem(ringModSourceCombos[idx1]->getIndex());
	ringModSourceCombos[idx1]->selectItem(type2);

}


//================================================================================================================================================================================================


void FlexFXAudioProcessorEditor::initChoruses()
{

	auto color = bdsp::NamedColorsIdentifier(FXTypeNames->operator[](FlexFXAudioProcessor::Chorus));
	for (int i = 0; i < numFXSlots; ++i)
	{
		chorusSections.add(new bdsp::Component(&GUIUniversals));
		auto chorusSection = chorusSections[i];
		auto name = juce::String(i + 1);

		GUIUniversals.hintBar->addTextToRecolor({ "Chorus " + name, color });
		GUIUniversals.hintBar->addTextToRecolor({ "Chorus " + name + "'s", color });

		//================================================================================================================================================================================================

		auto divParam = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.parameters->getParameter("Chorus" + name + "RateDivisionID"));
		chorusRateFracs.add(new bdsp::RangedSyncFraction(divParam, &GUIUniversals, topLevelComp->BPM.get(), name + "ChorusRate", true));
		auto currentFrac = chorusRateFracs[i];
		currentFrac->setFracColors(BDSP_COLOR_KNOB, color, BDSP_COLOR_DARK, BDSP_COLOR_LIGHT, color, color, color, color, color.withMultipliedAlpha(GUIUniversals.lowOpacity));
		currentFrac->getDivision()->setColorSchemeClassic(BDSP_COLOR_KNOB, bdsp::NamedColorsIdentifier(), bdsp::NamedColorsIdentifier(BDSP_COLOR_BLACK), color.withMultipliedAlpha(GUIUniversals.lowOpacity));
		currentFrac->attach(*audioProcessor.parameters.get(), "Chorus" + name + "Rate");

		chorusSection->addAndMakeVisible(currentFrac);

		currentFrac->setSpacing(0, 0.5, 0.5, 0.5, juce::RectanglePlacement::xLeft | juce::RectanglePlacement::yMid);

		//================================================================================================================================================================================================
		auto depthName = "Chorus " + name + " Depth";
		chorusDepthSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, depthName));
		bdsp::CircleSlider& currentDepth = *chorusDepthSliders[i];





		chorusSection->addAndMakeVisible(chorusDepthSliders[i]);
		currentDepth.attach(*audioProcessor.parameters.get(), currentDepth.getComponentID());

		currentDepth.setSliderColor(color, BDSP_COLOR_WHITE);
		currentDepth.label.setTitle(juce::String("Depth"));
		currentDepth.initText();

		currentDepth.setHintBarText("Intensity of Chorus " + name);

		//================================================================================================================================================================================================

		auto stereoName = "Chorus " + name + " Stereo";
		chorusStereoWidthSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, stereoName));
		bdsp::CircleSlider& currentstereo = *chorusStereoWidthSliders[i];





		chorusSection->addAndMakeVisible(chorusStereoWidthSliders[i]);
		currentstereo.attach(*audioProcessor.parameters.get(), currentstereo.getComponentID());

		currentstereo.setSliderColor(color, BDSP_COLOR_WHITE);
		currentstereo.label.setTitle(juce::String("Stereo"));
		currentstereo.initText();

		currentstereo.setHintBarText("Stereo width of Chorus " + name);


		//================================================================================================================================================================================================
		auto voiceName = "Chorus " + name + " Voices";
		chorusVoiceSliders.add(new bdsp::CircleSlider(&GUIUniversals, voiceName));
		auto currentVoice = chorusVoiceSliders[i];
		currentVoice->attach(*audioProcessor.parameters.get(), currentVoice->getComponentID());

		currentVoice->setSliderColor(color, BDSP_COLOR_WHITE);
		currentVoice->label.setTitle(juce::String("Voices"));
		currentVoice->initText();

		currentVoice->setHintBarText("Chorus " + name + "'s number of voices");
		chorusSection->addAndMakeVisible(currentVoice);



		//================================================================================================================================================================================================


		auto mixName = "FX " + name + " Mix";
		chorusMixSlider.add(new bdsp::RangedCircleSlider(&GUIUniversals, mixName));
		bdsp::CircleSlider& currentMix = *chorusMixSlider[i];





		chorusSection->addAndMakeVisible(chorusMixSlider[i]);
		currentMix.attach(*audioProcessor.parameters.get(), currentMix.getComponentID());

		currentMix.setSliderColor(color, BDSP_COLOR_WHITE);
		currentMix.label.setTitle(juce::String("Mix"));
		currentMix.initText();

		currentMix.setHintBarText("Dry/Wet mix of Chorus " + name);



		//================================================================================================================================================================================================
		chorusVisualizers.add(new bdsp::ChorusVisualizer(&GUIUniversals, dynamic_cast<FlexFXAudioProcessor*>(&audioProcessor)->choruses[i], &audioProcessor.lookups));
		chorusSection->addAndMakeVisible(chorusVisualizers[i]);
		auto currentVis = chorusVisualizers[i]->getVis();
		currentVis->setColor(color);
		currentVis->setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_BLACK);

		currentVis->depthParam = chorusDepthSliders[i]->getControlParamter();
		currentVis->stereoWidthParam = chorusStereoWidthSliders[i]->getControlParamter();
		currentVis->mixParam = chorusMixSlider[i]->getControlParamter();
		currentVis->numVoicesParam = chorusVoiceSliders[i]->getParameter();
	}
}
void FlexFXAudioProcessorEditor::resizeChoruses()
{

	auto row1 = 0.35;
	auto row2 = 0.35;
	auto visH = 0.3;
	auto bounds = fxSectionSize.toFloat();
	for (int i = 0; i < numFXSlots; ++i)
	{
		chorusSections[i]->setBounds(fxSectionSize);
		chorusRateFracs[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, 0, 2.0 / 3.0, row1)).reduced(GUIUniversals.rectThicc)));
		chorusMixSlider[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(2.0 / 3.0, 0, 1.0 / 3.0, row1)).reduced(GUIUniversals.rectThicc)));

		chorusDepthSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, row1, 1.0 / 3.0, row2)).reduced(GUIUniversals.rectThicc)));
		chorusStereoWidthSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(1.0 / 3.0, row1, 1.0 / 3.0, row2)).reduced(GUIUniversals.rectThicc)));
		chorusVoiceSliders[i]->setBoundsToIncludeLabel(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(2.0 / 3.0, row1, 1.0 / 3.0, row2)).reduced(GUIUniversals.rectThicc)));

		chorusVisualizers[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, row1 + row2, 1, visH)).reduced(GUIUniversals.rectThicc)));
	}
}

void FlexFXAudioProcessorEditor::swapChoruses(int idx1, int idx2)
{
	chorusRateFracs[idx1]->swap(chorusRateFracs[idx2]);

	chorusDepthSliders[idx1]->swap(chorusDepthSliders[idx2]);
	chorusStereoWidthSliders[idx1]->swap(chorusStereoWidthSliders[idx2]);
	chorusMixSlider[idx1]->swap(chorusMixSlider[idx2]);


	auto otherVoices = chorusVoiceSliders[idx2]->getValue();
	chorusVoiceSliders[idx2]->setValue(chorusVoiceSliders[idx1]->getValue(), juce::sendNotification);
	chorusVoiceSliders[idx1]->setValue(otherVoices, juce::sendNotification);

}


//================================================================================================================================================================================================


void FlexFXAudioProcessorEditor::initFlangers()
{
	auto color = bdsp::NamedColorsIdentifier(FXTypeNames->operator[](FlexFXAudioProcessor::Flanger));
	for (int i = 0; i < numFXSlots; ++i)
	{
		flangerSections.add(new bdsp::Component(&GUIUniversals));
		auto flangerSection = flangerSections[i];
		auto name = juce::String(i + 1);

		GUIUniversals.hintBar->addTextToRecolor({ "Flanger " + name, color });
		GUIUniversals.hintBar->addTextToRecolor({ "Flanger " + name + "'s", color });

		//================================================================================================================================================================================================

		auto divParam = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.parameters->getParameter("Flanger" + name + "RateDivisionID"));
		flangerRateFracs.add(new bdsp::RangedSyncFraction(divParam, &GUIUniversals, topLevelComp->BPM.get(), name + "FlangerRate", true));
		auto currentFrac = flangerRateFracs[i];
		currentFrac->setFracColors(BDSP_COLOR_KNOB, color, BDSP_COLOR_DARK, BDSP_COLOR_LIGHT, color, color, color, color, color.withMultipliedAlpha(GUIUniversals.lowOpacity));
		currentFrac->getDivision()->setColorSchemeClassic(BDSP_COLOR_KNOB, bdsp::NamedColorsIdentifier(), bdsp::NamedColorsIdentifier(BDSP_COLOR_BLACK), color.withMultipliedAlpha(GUIUniversals.lowOpacity));

		currentFrac->attach(*audioProcessor.parameters.get(), "Flanger" + name + "Rate");

		flangerSection->addAndMakeVisible(currentFrac);

		currentFrac->setSpacing(0, 0.5, 0.5, 0.5, juce::RectanglePlacement::xLeft | juce::RectanglePlacement::yMid);

		//================================================================================================================================================================================================
		auto baseName = "Flanger " + name + " Base";
		flangerBaseSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, baseName));
		bdsp::CircleSlider& currentBase = *flangerBaseSliders[i];





		flangerSection->addAndMakeVisible(flangerBaseSliders[i]);
		currentBase.attach(*audioProcessor.parameters.get(), currentBase.getComponentID());

		currentBase.setSliderColor(color, BDSP_COLOR_WHITE);
		currentBase.label.setTitle(juce::String("Base"));
		currentBase.initText();

		currentBase.setHintBarText("Base delay of Flanger " + name);
		//================================================================================================================================================================================================
		auto depthName = "Flanger " + name + " Depth";
		flangerDepthSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, depthName));
		bdsp::CircleSlider& currentDepth = *flangerDepthSliders[i];





		flangerSection->addAndMakeVisible(flangerDepthSliders[i]);
		currentDepth.attach(*audioProcessor.parameters.get(), currentDepth.getComponentID());

		currentDepth.setSliderColor(color, BDSP_COLOR_WHITE);
		currentDepth.label.setTitle(juce::String("Depth"));
		currentDepth.initText();

		currentDepth.setHintBarText("Depth of Flanger " + name + "'s delay change");

		//================================================================================================================================================================================================

		auto stereoName = "Flanger " + name + " Stereo";
		flangerStereoWidthSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, stereoName));
		bdsp::CircleSlider& currentstereo = *flangerStereoWidthSliders[i];





		flangerSection->addAndMakeVisible(flangerStereoWidthSliders[i]);
		currentstereo.attach(*audioProcessor.parameters.get(), currentstereo.getComponentID());

		currentstereo.setSliderColor(color, BDSP_COLOR_WHITE);
		currentstereo.label.setTitle(juce::String("Stereo"));
		currentstereo.initText();

		currentstereo.setHintBarText("Stereo width of Flanger " + name);

		//================================================================================================================================================================================================

		auto fbName = "Flanger " + name + " Feedback";
		flangerFeedbackSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, fbName));
		bdsp::CircleSlider& currentFB = *flangerFeedbackSliders[i];





		flangerSection->addAndMakeVisible(flangerFeedbackSliders[i]);
		currentFB.attach(*audioProcessor.parameters.get(), currentFB.getComponentID());

		currentFB.addCatchValues({ 0.5 });

		currentFB.setType(bdsp::BaseSlider::CenterZero);
		currentFB.setSliderColor(color, BDSP_COLOR_WHITE);
		currentFB.label.setTitle(juce::String("FB"));
		currentFB.initText();

		currentFB.setHintBarText("Feedback of Flanger " + name);

		//================================================================================================================================================================================================


		auto mixName = "FX " + name + " Mix";
		flangerMixSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, mixName));
		bdsp::CircleSlider& currentMix = *flangerMixSliders[i];





		flangerSection->addAndMakeVisible(flangerMixSliders[i]);
		currentMix.attach(*audioProcessor.parameters.get(), currentMix.getComponentID());

		currentMix.setSliderColor(color, BDSP_COLOR_WHITE);
		currentMix.label.setTitle(juce::String("Mix"));
		currentMix.initText();

		currentMix.setHintBarText("Dry/Wet mix of Flanger " + name);




		//================================================================================================================================================================================================
		flangerVisualizers.add(new bdsp::FlangerVisualizer(&GUIUniversals, dynamic_cast<FlexFXAudioProcessor*>(&audioProcessor)->flangers[i], &audioProcessor.lookups));
		auto currentVis = flangerVisualizers[i];
		flangerSection->addAndMakeVisible(currentVis);
		currentVis->setColor(color);
		currentVis->setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_BLACK);

		currentVis->baseParam = flangerBaseSliders[i]->getControlParamter();
		currentVis->depthParam = flangerDepthSliders[i]->getControlParamter();
		currentVis->stereoWidthParam = flangerStereoWidthSliders[i]->getControlParamter();
		currentVis->feedbackParam = flangerFeedbackSliders[i]->getControlParamter();
		currentVis->mixParam = flangerMixSliders[i]->getControlParamter();
	}
}

void FlexFXAudioProcessorEditor::resizeFlangers()
{
	auto row1 = 0.35;
	auto row2 = 0.35;
	auto visH = 0.3;
	auto bounds = fxSectionSize.toFloat();
	for (int i = 0; i < numFXSlots; ++i)
	{
		flangerSections[i]->setBounds(fxSectionSize);
		flangerRateFracs[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, 0, 2.0 / 3.0, row1)).reduced(GUIUniversals.rectThicc)));
		flangerMixSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(2.0 / 3.0, 0, 1.0 / 3.0, row1)).reduced(GUIUniversals.rectThicc)));


		flangerBaseSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, row1, 1.0 / 4.0, row2)).reduced(GUIUniversals.rectThicc)));
		flangerDepthSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(1.0 / 4.0, row1, 1.0 / 4.0, row2)).reduced(GUIUniversals.rectThicc)));
		flangerFeedbackSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(2.0 / 4.0, row1, 1.0 / 4.0, row2)).reduced(GUIUniversals.rectThicc)));
		flangerStereoWidthSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(3.0 / 4.0, row1, 1.0 / 4.0, row2)).reduced(GUIUniversals.rectThicc)));

		flangerVisualizers[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, row1 + row2, 1, visH)).reduced(GUIUniversals.rectThicc)));
	}
}

void FlexFXAudioProcessorEditor::swapFlangers(int idx1, int idx2)
{
	flangerRateFracs[idx1]->swap(flangerRateFracs[idx2]);

	flangerBaseSliders[idx1]->swap(flangerBaseSliders[idx2]);
	flangerDepthSliders[idx1]->swap(flangerDepthSliders[idx2]);
	flangerStereoWidthSliders[idx1]->swap(flangerStereoWidthSliders[idx2]);
	flangerFeedbackSliders[idx1]->swap(flangerFeedbackSliders[idx2]);
	flangerMixSliders[idx1]->swap(flangerMixSliders[idx2]);





}

//================================================================================================================================================================================================

void FlexFXAudioProcessorEditor::initPhasers()
{

	auto color = bdsp::NamedColorsIdentifier(FXTypeNames->operator[](FlexFXAudioProcessor::Phaser));
	for (int i = 0; i < numFXSlots; ++i)
	{
		phaserSections.add(new bdsp::Component(&GUIUniversals));
		auto phaserSection = phaserSections[i];
		auto name = juce::String(i + 1);

		GUIUniversals.hintBar->addTextToRecolor({ "Phaser " + name, color });
		GUIUniversals.hintBar->addTextToRecolor({ "Phaser " + name + "'s", color });

		//================================================================================================================================================================================================

		auto divParam = dynamic_cast<juce::AudioParameterChoice*>(audioProcessor.parameters->getParameter("Phaser" + name + "RateDivisionID"));
		phaserRateFracs.add(new bdsp::RangedSyncFraction(divParam, &GUIUniversals, topLevelComp->BPM.get(), name + "PhaserRate", true));
		auto currentFrac = phaserRateFracs[i];
		currentFrac->setFracColors(BDSP_COLOR_KNOB, color, BDSP_COLOR_DARK, BDSP_COLOR_LIGHT, color, color, color, color, color.withMultipliedAlpha(GUIUniversals.lowOpacity));
		currentFrac->getDivision()->setColorSchemeClassic(BDSP_COLOR_KNOB, bdsp::NamedColorsIdentifier(), bdsp::NamedColorsIdentifier(BDSP_COLOR_BLACK), color.withMultipliedAlpha(GUIUniversals.lowOpacity));

		currentFrac->attach(*audioProcessor.parameters.get(), "Phaser" + name + "Rate");

		phaserSection->addAndMakeVisible(currentFrac);

		currentFrac->setSpacing(0, 0.5, 0.5, 0.5, juce::RectanglePlacement::xLeft | juce::RectanglePlacement::yMid);
		//================================================================================================================================================================================================
		auto centerName = "Phaser " + name + " Center";
		phaserCenterSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, centerName));
		bdsp::CircleSlider& currentCenter = *phaserCenterSliders[i];





		phaserSection->addAndMakeVisible(phaserCenterSliders[i]);
		currentCenter.attach(*audioProcessor.parameters.get(), currentCenter.getComponentID());

		currentCenter.setSliderColor(color, BDSP_COLOR_WHITE);
		currentCenter.label.setTitle(juce::String("Center"));
		currentCenter.initText();

		currentCenter.setHintBarText("Center phase of Phaser " + name);
		//================================================================================================================================================================================================
		auto depthName = "Phaser " + name + " Depth";
		phaserDepthSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, depthName));
		bdsp::CircleSlider& currentDepth = *phaserDepthSliders[i];





		phaserSection->addAndMakeVisible(phaserDepthSliders[i]);
		currentDepth.attach(*audioProcessor.parameters.get(), currentDepth.getComponentID());

		currentDepth.setSliderColor(color, BDSP_COLOR_WHITE);
		currentDepth.label.setTitle(juce::String("Depth"));
		currentDepth.initText();

		currentDepth.setHintBarText("Depth of Phaser " + name + "'s delay change");

		//================================================================================================================================================================================================

		auto stereoName = "Phaser " + name + " Stereo";
		phaserStereoWidthSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, stereoName));
		bdsp::CircleSlider& currentstereo = *phaserStereoWidthSliders[i];





		phaserSection->addAndMakeVisible(phaserStereoWidthSliders[i]);
		currentstereo.attach(*audioProcessor.parameters.get(), currentstereo.getComponentID());

		currentstereo.setSliderColor(color, BDSP_COLOR_WHITE);
		currentstereo.label.setTitle(juce::String("Stereo"));
		currentstereo.initText();

		currentstereo.setHintBarText("Stereo width of Phaser " + name);

		//================================================================================================================================================================================================

		auto fbName = "Phaser " + name + " Feedback";
		phaserFeedbackSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, fbName));
		bdsp::CircleSlider& currentFB = *phaserFeedbackSliders[i];





		phaserSection->addAndMakeVisible(phaserFeedbackSliders[i]);
		currentFB.attach(*audioProcessor.parameters.get(), currentFB.getComponentID());
		currentFB.addCatchValues({ 0.5 });
		currentFB.setSliderColor(color, BDSP_COLOR_WHITE);
		currentFB.label.setTitle(juce::String("FB"));
		currentFB.initText();

		currentFB.setHintBarText("Feedback of Phaser " + name);



		//================================================================================================================================================================================================


		auto mixName = "FX " + name + " Mix";
		phaserMixSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, mixName));
		bdsp::CircleSlider& currentMix = *phaserMixSliders[i];





		phaserSection->addAndMakeVisible(phaserMixSliders[i]);
		currentMix.attach(*audioProcessor.parameters.get(), currentMix.getComponentID());

		currentMix.setSliderColor(color, BDSP_COLOR_WHITE);
		currentMix.label.setTitle(juce::String("Mix"));
		currentMix.initText();

		currentMix.setHintBarText("Dry/Wet mix of Phaser " + name);



		//================================================================================================================================================================================================
		phaserVisualizers.add(new bdsp::PhaserVisualizer(&GUIUniversals, dynamic_cast<FlexFXAudioProcessor*>(&audioProcessor)->phasers[i], &audioProcessor.lookups));
		auto currentVis = phaserVisualizers[i];
		phaserSection->addAndMakeVisible(currentVis);
		currentVis->setColor(color);
		currentVis->setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_BLACK);

		currentVis->centerParam = phaserCenterSliders[i]->getControlParamter();
		currentVis->depthParam = phaserDepthSliders[i]->getControlParamter();
		currentVis->stereoWidthParam = phaserStereoWidthSliders[i]->getControlParamter();
		currentVis->feedbackParam = phaserFeedbackSliders[i]->getControlParamter();
		currentVis->mixParam = phaserMixSliders[i]->getControlParamter();
	}
}

void FlexFXAudioProcessorEditor::resizePhasers()
{
	auto row1 = 0.35;
	auto row2 = 0.35;
	auto visH = 0.3;
	auto bounds = fxSectionSize.toFloat();
	for (int i = 0; i < numFXSlots; ++i)
	{
		phaserSections[i]->setBounds(fxSectionSize);
		phaserRateFracs[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, 0, 2.0 / 3.0, row1)).reduced(GUIUniversals.rectThicc)));
		phaserMixSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(2.0 / 3.0, 0, 1.0 / 3.0, row1)).reduced(GUIUniversals.rectThicc)));

		phaserCenterSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, row1, 0.25, row2)).reduced(GUIUniversals.rectThicc)));
		phaserDepthSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.25, row1, 0.25, row2)).reduced(GUIUniversals.rectThicc)));

		phaserStereoWidthSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.5, row1, 0.25, row2)).reduced(GUIUniversals.rectThicc)));
		phaserFeedbackSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.75, row1, 0.25, row2)).reduced(GUIUniversals.rectThicc)));

		phaserVisualizers[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, row1 + row2, 1, visH)).reduced(GUIUniversals.rectThicc)));
	}
}

void FlexFXAudioProcessorEditor::swapPhasers(int idx1, int idx2)
{
	phaserRateFracs[idx1]->swap(phaserRateFracs[idx2]);

	phaserCenterSliders[idx1]->swap(phaserCenterSliders[idx2]);
	phaserDepthSliders[idx1]->swap(phaserDepthSliders[idx2]);
	phaserStereoWidthSliders[idx1]->swap(phaserStereoWidthSliders[idx2]);
	phaserFeedbackSliders[idx1]->swap(phaserFeedbackSliders[idx2]);
	phaserMixSliders[idx1]->swap(phaserMixSliders[idx2]);

	//auto otherStages = phaserStageSliders[idx2]->getValue();
	//phaserStageSliders[idx2]->setValue(phaserStageSliders[idx1]->getValue(), juce::sendNotification);
	//phaserStageSliders[idx1]->setValue(otherStages, juce::sendNotification);
}




//================================================================================================================================================================================================

void FlexFXAudioProcessorEditor::initPanners()
{

	auto color = bdsp::NamedColorsIdentifier(FXTypeNames->operator[](FlexFXAudioProcessor::Panner));
	for (int i = 0; i < numFXSlots; ++i)
	{
		pannerSections.add(new bdsp::Component(&GUIUniversals));
		auto PannerSection = pannerSections[i];
		auto name = juce::String(i + 1);

		GUIUniversals.hintBar->addTextToRecolor({ "Panner " + name, color });
		GUIUniversals.hintBar->addTextToRecolor({ "Panner " + name + "'s", color });

		//================================================================================================================================================================================================


		auto gainName = "Panner " + name + " Gain";
		pannerGainSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, gainName));
		bdsp::CircleSlider& currentGain = *pannerGainSliders[i];





		PannerSection->addAndMakeVisible(pannerGainSliders[i]);
		currentGain.attach(*audioProcessor.parameters.get(), currentGain.getComponentID());

		currentGain.setSliderColor(color, BDSP_COLOR_WHITE);
		currentGain.label.setTitle(juce::String("Gain"));
		currentGain.initText();

		currentGain.setHintBarText("Gain of Panner " + name);

		//================================================================================================================================================================================================


		auto panName = "Panner " + name + " Pan";
		pannerPanSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, panName));
		bdsp::CircleSlider& currentPan = *pannerPanSliders[i];





		PannerSection->addAndMakeVisible(pannerPanSliders[i]);
		currentPan.attach(*audioProcessor.parameters.get(), currentPan.getComponentID());

		currentPan.setSliderColor(color, BDSP_COLOR_WHITE);
		currentPan.label.setTitle(juce::String("Pan"));
		currentPan.initText();

		currentPan.setHintBarText("Pan of Panner " + name);

		//================================================================================================================================================================================================


		auto stereoName = "Panner " + name + " Stereo Width";
		pannerWidthSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, stereoName));
		bdsp::CircleSlider& currentstereo = *pannerWidthSliders[i];





		PannerSection->addAndMakeVisible(pannerWidthSliders[i]);
		currentstereo.attach(*audioProcessor.parameters.get(), currentstereo.getComponentID());

		currentstereo.setSliderColor(color, BDSP_COLOR_WHITE);
		currentstereo.label.setTitle(juce::String("Stereo"));
		currentstereo.initText();

		currentstereo.setHintBarText("Stereo width of Panner " + name);

		//================================================================================================================================================================================================




		//================================================================================================================================================================================================


		auto mixName = "FX " + name + " Mix";
		pannerMixSliders.add(new bdsp::RangedCircleSlider(&GUIUniversals, mixName));
		bdsp::CircleSlider& currentMix = *pannerMixSliders[i];





		PannerSection->addAndMakeVisible(pannerMixSliders[i]);
		currentMix.attach(*audioProcessor.parameters.get(), currentMix.getComponentID());

		currentMix.setSliderColor(color, BDSP_COLOR_WHITE);
		currentMix.label.setTitle(juce::String("Mix"));
		currentMix.initText();

		currentMix.setHintBarText("Dry/Wet mix of Panner " + name);



		//================================================================================================================================================================================================
		pannerMeters.add(new bdsp::LevelMeter<float>(&GUIUniversals, dynamic_cast<FlexFXAudioProcessor*>(&audioProcessor)->pannerMeterControllers[i]));
		auto currentVis = pannerMeters[i];
		PannerSection->addAndMakeVisible(currentVis);
		currentVis->setColors(color);
		currentVis->setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_BLACK);

	}
}

void FlexFXAudioProcessorEditor::resizePanners()
{
	auto row1 = 0.35;
	auto row2 = 0.35;
	auto visH = 0.3;
	auto bounds = fxSectionSize.toFloat();
	for (int i = 0; i < numFXSlots; ++i)
	{
		pannerSections[i]->setBounds(fxSectionSize);
		pannerGainSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, 0, 0.5, row1)).reduced(GUIUniversals.rectThicc)));
		pannerMixSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.5, 0, 0.5, row1)).reduced(GUIUniversals.rectThicc)));
		pannerPanSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, row1, 0.5, row2)).reduced(GUIUniversals.rectThicc)));
		pannerWidthSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.5, row1, 0.5, row2)).reduced(GUIUniversals.rectThicc)));

		pannerMeters[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, row1 + row2, 1, visH)).reduced(GUIUniversals.rectThicc)));
	}
}

void FlexFXAudioProcessorEditor::swapPanners(int idx1, int idx2)
{
	pannerGainSliders[idx1]->swap(pannerGainSliders[idx2]);
	pannerPanSliders[idx1]->swap(pannerPanSliders[idx2]);
	pannerWidthSliders[idx1]->swap(pannerWidthSliders[idx2]);
	pannerMixSliders[idx1]->swap(pannerMixSliders[idx2]);
}



//================================================================================================================================================================================================

void FlexFXAudioProcessorEditor::initNoises()
{

	auto color = bdsp::NamedColorsIdentifier(FXTypeNames->operator[](FlexFXAudioProcessor::Noise));
	for (int i = 0; i < numFXSlots; ++i)
	{
		NoiseSections.add(new bdsp::Component(&GUIUniversals));
		auto NoiseSection = NoiseSections[i];
		auto name = juce::String(i + 1);

		GUIUniversals.hintBar->addTextToRecolor({ "Noise " + name, color });
		GUIUniversals.hintBar->addTextToRecolor({ "Noise " + name + "'s", color });

		//================================================================================================================================================================================================

		auto gain = createRangedCircleSlider("Noise " + name + " Gain", NoiseSection, noiseGainSliders, color, "Gain");

		gain->slider.setHintBarText("Gain of Noise " + name);


		auto c = createRangedCircleSlider("Noise " + name + " Color", NoiseSection, noiseColorSliders, color, "Color");

		c->slider.setHintBarText("Color of Noise " + name);
		c->slider.addCatchValues({ 0 }, false);
		c->slider.setType(bdsp::BaseSlider::CenterZero);

		auto stereo = createRangedCircleSlider("Noise " + name + " Stereo", NoiseSection, noiseStereoSliders, color, "Stereo Width");

		stereo->slider.setHintBarText("Stereo Width of Noise " + name);





		auto dry = createRangedCircleSlider("Noise " + name + " Dry Gain", NoiseSection, noiseDrySliders, color, "Dry Gain");

		dry->slider.setHintBarText("Gain of input signal");



		noiseVisualizers.add(new bdsp::NoiseVisualizer(&GUIUniversals));
		noiseVisualizers.getLast()->getVis()->gainParam = gain->getControlParamter();
		noiseVisualizers.getLast()->getVis()->colorParam = c->getControlParamter();
		//noiseVisualizers.getLast()->getVis()->mixParam = dry->getControlParamter();

		noiseVisualizers.getLast()->setColor(color);

		NoiseSection->addAndMakeVisible(noiseVisualizers.getLast());

	}
}

void FlexFXAudioProcessorEditor::resizeNoises()
{
	auto row1 = 0.35;
	auto row2 = 0.35;
	auto visH = 0.3;
	auto bounds = fxSectionSize.toFloat();
	for (int i = 0; i < numFXSlots; ++i)
	{
		NoiseSections[i]->setBounds(fxSectionSize);


		noiseGainSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.5, 0, 0.5, row1)).reduced(GUIUniversals.rectThicc)));
		noiseDrySliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, 0, 0.5, row1)).reduced(GUIUniversals.rectThicc)));

		noiseColorSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, row1, 0.5, row2)).reduced(GUIUniversals.rectThicc)));
		noiseStereoSliders[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0.5, row1, 0.5, row2)).reduced(GUIUniversals.rectThicc)));



		noiseVisualizers[i]->setBounds(bdsp::shrinkRectangleToInt(bounds.getProportion(juce::Rectangle<float>(0, row1 + row2, 1, visH)).reduced(GUIUniversals.rectThicc)));


	}
}

void FlexFXAudioProcessorEditor::swapNoises(int idx1, int idx2)
{
	noiseGainSliders[idx1]->swap(noiseGainSliders[idx2]);
	noiseColorSliders[idx1]->swap(noiseColorSliders[idx2]);
	noiseStereoSliders[idx1]->swap(noiseStereoSliders[idx2]);
	noiseDrySliders[idx1]->swap(noiseDrySliders[idx2]);



}







//================================================================================================================================================================================================

FlexFXAudioProcessorEditor::FXSlot::FXSlot(FlexFXAudioProcessorEditor* editor, int slotNum, bdsp::Button* chainBypassButton)
	:bdsp::Component(&editor->GUIUniversals),
	bdsp::GUI_Universals::Listener(&editor->GUIUniversals),
	fxSectionHolder(&editor->GUIUniversals)
{


	p = editor;
	title = juce::String(slotNum + 1);
	addAndMakeVisible(fxSectionHolder);


	slotIndex = slotNum;


	//================================================================================================================================================================================================

	juce::Array<std::pair<juce::Range<int>, int>> splits({
		std::pair<juce::Range<int>, int>(juce::Range<int>(0,7), 0),
		std::pair<juce::Range<int>, int>(juce::Range<int>(7,12), 1) });



	fxTypeSelector = std::make_unique<bdsp::IncrementalComboBox<>>(dynamic_cast<juce::AudioParameterChoice*>(p->audioProcessor.parameters->getParameter("FX" + title + "TypeID")), &p->GUIUniversals);
	addAndMakeVisible(fxTypeSelector.get());

	juce::Array<bdsp::NamedColorsIdentifier> colors;
	for (int i = 0; i < p->FXTypeNames->size(); ++i)
	{
		colors.set(i, bdsp::NamedColorsIdentifier(p->FXTypeNames->operator[](i)));
	}
	fxTypeSelector->setFontIndex(3);
	fxTypeSelector->getListComponent()->justification = juce::Justification::centredLeft;
	fxTypeSelector->getListComponent()->setColSplits(splits);
	fxTypeSelector->setColorSchemeMinimal(bdsp::NamedColorsIdentifier(), BDSP_COLOR_PURE_BLACK, BDSP_COLOR_DARK, colors, BDSP_COLOR_DARK);
	fxTypeSelector->getListComponent()->heightRatio = 0.1;
	fxTypeSelector->setMenuWidth(1.1);
	fxTypeSelector->onChange = [=](int idx)
	{
		setFXType(idx);
		fxTypeSelector->updateIncrementArrows(idx);
	};

	//================================================================================================================================================================================================


	bypassButton = std::make_unique<bdsp::PathButton>(universals);
	bypassButton->setClickingTogglesState(true);
	bypassButton->setPathBorder(0.1);
	bypassButton->setPath(universals->commonPaths.powerSymbol);
	addAndMakeVisible(bypassButton.get());

	bypassButton->onStateChange = [=]()
	{
		bool newState = !(bypassButton->getToggleState() || chainBypassButton->getToggleState());
		fxSectionHolder.setEnabled(newState);
		fxTypeSelector->setEnabled(newState);
	};

	bypassButton->attach(*p->audioProcessor.parameters, "FX" + title + "BypassID");



}


void FlexFXAudioProcessorEditor::FXSlot::paint(juce::Graphics& g)
{
	g.setColour(getColor(BDSP_COLOR_MID));
	g.fillRoundedRectangle(getLocalBounds().toFloat(), universals->roundedRectangleCurve);



	g.setColour(getColor(BDSP_COLOR_PURE_BLACK));
	g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(universals->rectThicc / 3), universals->roundedRectangleCurve);

	auto dividerY = (bypassButton->getBottom() + fxTypeSelector->getY()) / 2.0;
	//	bdsp::drawDivider(g, juce::Line<float>(borderColor, dividerY, getWidth() - borderColor, dividerY), getColor(BDSP_COLOR_MID), universals->dividerSize);
	g.setColour(getColor(BDSP_COLOR_BLACK));
	g.drawHorizontalLine(dividerY, border, getWidth() - border);

	g.setColour(getColor(currentColor));
	//bdsp::drawText(g, universals->Fonts[2].getFont(), title, getLocalBounds().withBottom(dividerY), true, juce::Justification::centred, 0.5);

	g.setColour(getColor(BDSP_COLOR_BLACK));
	g.fillRoundedRectangle(fxSectionHolder.getBounds().toFloat(), universals->roundedRectangleCurve);
}

void FlexFXAudioProcessorEditor::FXSlot::resized()
{
	border = universals->rectThicc;
	auto bounds = getLocalBounds().toFloat();
	auto topBar = bounds.getProportion(juce::Rectangle<float>(0, 0, 1, 0.1)).withTrimmedTop(border).reduced(border, 0);
	auto typeBar = topBar.withY(topBar.getBottom());
	auto mainArea = juce::Rectangle<float>().leftTopRightBottom(typeBar.getX(), typeBar.getBottom(), typeBar.getRight(), getHeight() - border);


	bypassButton->setBounds(juce::Rectangle<int>(topBar.getHeight(), topBar.getHeight()).translated(border, 0).reduced(border));
	//fxTypeSelector->setBounds(bdsp::shrinkRectangleToInt(typeBar.reduced(borderColor)));

	fxTypeSelector->setBounds(bdsp::shrinkRectangleToInt(typeBar).reduced(border, 0));

	fxSectionHolder.setBounds(bdsp::shrinkRectangleToInt(mainArea.reduced(border)));
	if (currentFXSection != nullptr)
	{
		currentFXSection->setBounds(fxSectionHolder.getLocalBounds());
	}

}

void FlexFXAudioProcessorEditor::FXSlot::setFXType(int idx)
{
	currentColor = p->FXTypeNames->operator[](idx);

	bypassButton->setColor(currentColor.withMultipliedAlpha(universals->midOpacity), currentColor);

	int i = p->fxSlotManager->getRearrangeComp()->findDraggerIndex(slotIndex);
	p->fxSlotManager->setDragColor(i, currentColor);
	p->fxSlotManager->setDragText(i, currentColor.ID.toString());


	fxSectionHolder.removeAllChildren();
	currentFXSection = nullptr;
	switch (idx)
	{
	case FlexFXAudioProcessor::Distortion:
		currentFXSection = (p->distortionSections[slotIndex]);
		break;
	case FlexFXAudioProcessor::BitCrush:
		currentFXSection = (p->bitcrushSections[slotIndex]);
		break;
	case FlexFXAudioProcessor::Filter:
		currentFXSection = (p->filterSections[slotIndex]);
		break;
	case FlexFXAudioProcessor::EQ:
		currentFXSection = (p->EQSections[slotIndex]);
		break;
	case FlexFXAudioProcessor::PitchShift:
		currentFXSection = (p->pitchShiftSections[slotIndex]);
		break;
	case FlexFXAudioProcessor::RingMod:
		currentFXSection = (p->ringModSections[slotIndex]);
		break;
	case FlexFXAudioProcessor::Chorus:
		currentFXSection = (p->chorusSections[slotIndex]);
		break;
	case FlexFXAudioProcessor::Flanger:
		currentFXSection = (p->flangerSections[slotIndex]);
		break;
	case FlexFXAudioProcessor::Phaser:
		currentFXSection = (p->phaserSections[slotIndex]);
		break;
	case FlexFXAudioProcessor::Panner:
		currentFXSection = (p->pannerSections[slotIndex]);
		break;
	case FlexFXAudioProcessor::Noise:
		currentFXSection = (p->NoiseSections[slotIndex]);
		break;
	default:
		break;
	}

	fxSectionHolder.addAndMakeVisible(currentFXSection);


	//p->routing->setColor(slotIndex, currentColor);

	repaint();
}

void FlexFXAudioProcessorEditor::FXSlot::visibilityChanged()
{
	if (currentFXSection != nullptr)
	{
		currentFXSection->setVisible(isVisible());

		for (auto c : currentFXSection->getChildren())
		{
			c->setVisible(isVisible());
		}
	}
}




int FlexFXAudioProcessorEditor::FXSlot::getSlotIndex() const
{
	return slotIndex;
}

void FlexFXAudioProcessorEditor::FXSlot::GUI_UniversalsChanged()
{
	repaint();
}

juce::Rectangle<float> FlexFXAudioProcessorEditor::FXSlot::getDraghandleRect()
{
	return juce::Rectangle<float>(2 * bypassButton->getHeight(), bypassButton->getHeight()).withCentre(juce::Point<float>(getWidth() / 2.0f, bypassButton->getBounds().getCentreY()));
	//return juce::Rectangle<float>().leftTopRightBottom(bypassButton->getRight(), bypassButton->getY(), getWidth() - bypassButton->getX(), bypassButton->getBottom());
}

juce::Rectangle<int> FlexFXAudioProcessorEditor::FXSlot::getFXSectionBounds()
{
	return fxSectionHolder.getBounds();
}

void FlexFXAudioProcessorEditor::FXSlot::swapWithOtherSlot(FXSlot* otherSlot)
{

	// swap each fx reagrdless of if selected
	p->swapFX(slotIndex, otherSlot->slotIndex);


	//================================================================================================================================================================================================


	auto otherColor = otherSlot->currentColor;

	auto otherBypassState = otherSlot->bypassButton->getToggleState();

	auto otherTypeInt = otherSlot->fxTypeSelector->getIndex();


	otherSlot->currentColor = currentColor;
	currentColor = otherColor;

	otherSlot->bypassButton->setToggleState(bypassButton->getToggleState(), juce::sendNotification);
	bypassButton->setToggleState(otherBypassState, juce::sendNotification);

	otherSlot->fxTypeSelector->selectItem(fxTypeSelector->getIndex());
	fxTypeSelector->selectItem(otherTypeInt);



	//================================================================================================================================================================================================



	repaint();
	otherSlot->repaint();


}


//================================================================================================================================================================================================


FlexFXAudioProcessorEditor::AnimatedTitle::AnimatedTitle(FlexFXAudioProcessorEditor* parent)
	:bdsp::OpenGLComponent(&parent->GUIUniversals)
{
	e = parent;
	e->titleBar->addAndMakeVisible(this);

	background = BDSP_COLOR_DARK;
	vertexBuffer.resize(4);
	indexBuffer.addArray({
		0,1,2,
		1,2,3
		});

	auto prev = getColor(e->FXTypeNames->operator[](0));
	juce::Array<juce::Colour> colors;

	for (int i = 1; i < e->FXTypeNames->size(); ++i)
	{
		const auto& c = getColor(e->FXTypeNames->operator[](i));
		if (c != prev)
		{
			colors.add(c);
			prev = c;
		}
	}

	auto comparator = bdsp::ColorComparator(bdsp::colorChannel::Hue);
	colors.sort(comparator);

	for (const auto& c : colors)
	{
		reds.add(c.getFloatRed());
		greens.add(c.getFloatGreen());
		blues.add(c.getFloatBlue());
	}
}

FlexFXAudioProcessorEditor::AnimatedTitle::~AnimatedTitle()
{
	universals->contextHolder->unregisterOpenGlRenderer(this);
}

void FlexFXAudioProcessorEditor::AnimatedTitle::paintOverChildren(juce::Graphics& g)
{
	g.setColour(getColor(background));
	g.fillPath(cutout);
}

void FlexFXAudioProcessorEditor::AnimatedTitle::resized()
{
	auto logo = e->topLevelComp->logo->getPath();
	juce::Path p, rect;
	juce::GlyphArrangement ga;
	ga.addLineOfText(universals->Fonts[2].getFont(), "Flex-FX", 0, 0);
	ga.createPath(p);

	bdsp::scaleToFit(p, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(0.25, 0.1, 0.75, 0.8)));
	bdsp::scaleToFit(logo, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(0, 0.1, 0.25, 0.8)));


	auto translation = juce::AffineTransform().translated(-logo.getBounds().getX() / 2, 0);
	logo.applyTransform(translation);
	p.applyTransform(translation);

	p.addPath(logo);


	rect.addRectangle(getLocalBounds());

	cutout = ClipperLib::performBoolean(rect, p, ClipperLib::ctDifference);

}

void FlexFXAudioProcessorEditor::AnimatedTitle::generateVertexBuffer()
{
	float tmp;
	colorProgress = modff(colorProgress + colorProgressInc, &tmp);
	rotationProgress = modff(rotationProgress + rotationProgressInc, &tmp);

	int index1 = int(floor(colorProgress * (reds.size()))) % reds.size();
	int index2 = (index1 + 1) % reds.size();
	int index3 = (index2 + 1) % reds.size();

	float prop = colorProgress * (reds.size()) - floor(colorProgress * (reds.size()));


	auto r1 = reds[index1] * (1.0f - prop) + reds[index2] * prop;
	auto r2 = reds[index2] * (1.0f - prop) + reds[index3] * prop;
	auto r12 = (r1 + r2) / 2;

	auto g1 = greens[index1] * (1.0f - prop) + greens[index2] * prop;
	auto g2 = greens[index2] * (1.0f - prop) + greens[index3] * prop;
	auto g12 = (g1 + g2) / 2;

	auto b1 = blues[index1] * (1.0f - prop) + blues[index2] * prop;
	auto b2 = blues[index2] * (1.0f - prop) + blues[index3] * prop;
	auto b12 = (b1 + b2) / 2;


	float rA, rB, rC, rD;
	float gA, gB, gC, gD;
	float bA, bB, bC, bD;

	if (rotationProgress <= 0.25f)
	{
		float rotationProp = rotationProgress * 4;
		rA = r1 * (1 - rotationProp) + r12 * rotationProp;
		rB = r12 * (1 - rotationProp) + r2 * rotationProp;
		rC = r12 * (1 - rotationProp) + r1 * rotationProp;
		rD = r2 * (1 - rotationProp) + r12 * rotationProp;



		gA = g1 * (1 - rotationProp) + g12 * rotationProp;
		gB = g12 * (1 - rotationProp) + g2 * rotationProp;
		gC = g12 * (1 - rotationProp) + g1 * rotationProp;
		gD = g2 * (1 - rotationProp) + g12 * rotationProp;


		bA = b1 * (1 - rotationProp) + b12 * rotationProp;
		bB = b12 * (1 - rotationProp) + b2 * rotationProp;
		bC = b12 * (1 - rotationProp) + b1 * rotationProp;
		bD = b2 * (1 - rotationProp) + b12 * rotationProp;
	}
	else if (rotationProgress <= 0.5f)
	{
		float rotationProp = (rotationProgress - 0.25f) * 4;
		rA = r12 * (1 - rotationProp) + r2 * rotationProp;
		rB = r2 * (1 - rotationProp) + r12 * rotationProp;
		rC = r1 * (1 - rotationProp) + r12 * rotationProp;
		rD = r12 * (1 - rotationProp) + r1 * rotationProp;

		gA = g12 * (1 - rotationProp) + g2 * rotationProp;
		gB = g2 * (1 - rotationProp) + g12 * rotationProp;
		gC = g1 * (1 - rotationProp) + g12 * rotationProp;
		gD = g12 * (1 - rotationProp) + g1 * rotationProp;


		bA = b12 * (1 - rotationProp) + b2 * rotationProp;
		bB = b2 * (1 - rotationProp) + b12 * rotationProp;
		bC = b1 * (1 - rotationProp) + b12 * rotationProp;
		bD = b12 * (1 - rotationProp) + b1 * rotationProp;
	}
	else if (rotationProgress <= 0.75f)
	{
		float rotationProp = (rotationProgress - 0.5f) * 4;


		rA = r2 * (1 - rotationProp) + r12 * rotationProp;
		rB = r12 * (1 - rotationProp) + r1 * rotationProp;
		rC = r12 * (1 - rotationProp) + r2 * rotationProp;
		rD = r1 * (1 - rotationProp) + r12 * rotationProp;

		gA = g2 * (1 - rotationProp) + g12 * rotationProp;
		gB = g12 * (1 - rotationProp) + g1 * rotationProp;
		gC = g12 * (1 - rotationProp) + g2 * rotationProp;
		gD = g1 * (1 - rotationProp) + g12 * rotationProp;

		bA = b2 * (1 - rotationProp) + b12 * rotationProp;
		bB = b12 * (1 - rotationProp) + b1 * rotationProp;
		bC = b12 * (1 - rotationProp) + b2 * rotationProp;
		bD = b1 * (1 - rotationProp) + b12 * rotationProp;

	}
	else
	{
		float rotationProp = (rotationProgress - 0.75f) * 4;


		rA = r12 * (1 - rotationProp) + r1 * rotationProp;
		rB = r1 * (1 - rotationProp) + r12 * rotationProp;
		rC = r2 * (1 - rotationProp) + r12 * rotationProp;
		rD = r12 * (1 - rotationProp) + r2 * rotationProp;

		gA = g12 * (1 - rotationProp) + g1 * rotationProp;
		gB = g1 * (1 - rotationProp) + g12 * rotationProp;
		gC = g2 * (1 - rotationProp) + g12 * rotationProp;
		gD = g12 * (1 - rotationProp) + g2 * rotationProp;


		bA = b12 * (1 - rotationProp) + b1 * rotationProp;
		bB = b1 * (1 - rotationProp) + b12 * rotationProp;
		bC = b2 * (1 - rotationProp) + b12 * rotationProp;
		bD = b12 * (1 - rotationProp) + b2 * rotationProp;



	}

	vertexBuffer.set(0, {

		-1.0f,1.0f,
		rA,gA,bA,1.0f
		});

	vertexBuffer.set(1, {

		-1.0f,-1.0f,
		rB,gB,bB,1.0f
		});


	vertexBuffer.set(2, {

		1.0f,1.0f,
		rC,gC,bC,1.0f
		});


	vertexBuffer.set(3, {

		1.0f,-1.0f,
		rD,gD,bD,1.0f
		});



}

FlexFXAudioProcessorEditor::FXChainManager::FXChainManager(FlexFXAudioProcessorEditor* editor)
	:bdsp::RearrangeableTabsComponent(&editor->GUIUniversals, juce::StringArray({ "FX A", "FX B" }))
{
	p = editor;
	setHasCutout(true, 1.75);

	isVertical = false;
	rearrangeComp->instantSwap = false;
	tabRatio = 0.2;
	scaleTitles = false;

	bkgd = BDSP_COLOR_DARK;

	onTabChange = [=](int t)
	{
		for (int i = 0; i < rearrangeComp->comps.size(); ++i)
		{
			rearrangeComp->comps[i]->setVisible(i / numPerTab == t);
		}
	};

	rearrangeComp->onComponentOrderChanged = [=](int indexMoved, int indexMovedTo)
	{
		compCasts[indexMoved]->swapWithOtherSlot(compCasts[indexMovedTo]);

		resized();

	};



	swapOrder = std::make_unique<bdsp::PathButton>(universals);

	swapOrder->setPath(universals->commonPaths.swap);

	swapOrder->setColor(BDSP_COLOR_WHITE);

	swapOrder->onClick = [=]()
	{
		for (int i = 0; i < numFXPerChain; ++i)
		{
			rearrangeComp->onComponentOrderChanged(i, numFXPerChain + i);
		}

		chainBypassButtons[0]->swapToggleStates(chainBypassButtons[1]);

	};

	addAndMakeVisible(swapOrder.get());


	parallelMixSlider = std::make_unique<bdsp::RangedContainerSlider>(&p->GUIUniversals, "Parallel Mix");

	parallelMixSlider->slider.setType(bdsp::BaseSlider::Type::CenterZero);

	parallelMixSlider->slider.attach(*editor->audioProcessor.parameters.get(), "ParallelMixID");

	parallelMixSlider->slider.setSliderColors(bdsp::NamedColorsIdentifier(), BDSP_COLOR_WHITE, bdsp::NamedColorsIdentifier(), bdsp::NamedColorsIdentifier());

	addChildComponent(parallelMixSlider.get());

	routingButton = std::make_unique<bdsp::TextButton>(&p->GUIUniversals);
	routingButton->setText("Parallel", "Series");
	routingButton->setClickingTogglesState(true);

	routingButton->backgroundDown = bdsp::NamedColorsIdentifier();
	routingButton->backgroundUp = bdsp::NamedColorsIdentifier();
	routingButton->textDown = BDSP_COLOR_WHITE;
	routingButton->textUp = BDSP_COLOR_WHITE;

	routingButton->setTextHeightRatio(0.75);

	routingButton->onClick = [=]()
	{
		swapOrder->setVisible(!routingButton->getToggleState());
		parallelMixSlider->setVisible(routingButton->getToggleState());
	};

	routingButton->attach(*editor->audioProcessor.parameters.get(), "RoutingID");
	addAndMakeVisible(routingButton.get());


	//================================================================================================================================================================================================

	for (int i = 0; i < numFXChains; ++i)
	{
		chainBypassButtons.add(new bdsp::PathButton(universals, true));
		chainBypassButtons.getLast()->setPath(universals->commonPaths.powerSymbol);
		chainBypassButtons.getLast()->setColor(bdsp::NamedColorsIdentifier(BDSP_COLOR_WHITE).withMultipliedAlpha(universals->disabledAlpha), BDSP_COLOR_WHITE);

		chainBypassButtons[i]->onStateChange = [=]()
		{
			for (int j = 0; j < numFXPerChain && compCasts.size() >= (i + 1) * numFXPerChain; ++j)
			{
				//	bool newState = !(chainBypassButtons[i]->getToggleState() || compCasts[i * numFXPerChain + j]->getBypassButton()->getToggleState());
				//	compCasts[i * numFXPerChain + j]->getfxSectionHolder()->setEnabled(newState);
				//	compCasts[i * numFXPerChain + j]->fxTypeSelector->setEnabled(newState);
				compCasts[i * numFXPerChain + j]->getBypassButton()->onStateChange();
			}
		};

		chainBypassButtons.getLast()->attach(*editor->audioProcessor.parameters.get(), "Chain" + bdsp::asciiCharToJuceString(65 + i) + "BypassID");
		addAndMakeVisible(chainBypassButtons.getLast());

	}

}


void FlexFXAudioProcessorEditor::FXChainManager::resized()
{
	bdsp::RearrangeableTabsComponent::resized();


	routingButton->setBounds(cutoutRect.getProportion(juce::Rectangle<float>(0, 0.5, 1, 0.5)));
	swapOrder->setBounds(cutoutRect.getProportion(juce::Rectangle<float>(0, 0, 1, 0.5)));
	parallelMixSlider->setBounds(cutoutRect.getProportion(juce::Rectangle<float>(0, 0, 1, 0.5)));


	int j = 0;
	for (int t = 0; t < buttons.size(); t++)
	{
		buttons[t]->setTextBorder(juce::BorderSize<int>(universals->rectThicc));
		auto textBounds = buttons[t]->getTextBounds();
		auto bypassRect = juce::Rectangle<float>(textBounds.getHeight() * 0.8, textBounds.getHeight() * 0.8).withCentre({ textBounds.getCentreX(),textBounds.getCentreY() }).withRightX(textBounds.getX() - textBounds.getHeight() * 0.5);

		chainBypassButtons[t]->setBounds(bdsp::shrinkRectangleToInt(bypassRect.translated(buttons[t]->getX(), buttons[t]->getY())));
		for (int i = 0; i < numPerTab; ++i)
		{
			rearrangeComp->comps[j]->setBounds(pages.getFirst()->getLocalBounds().getProportion(juce::Rectangle<float>(i / float(numPerTab), 0, 1.0 / numPerTab, 1)).reduced(universals->rectThicc));
			++j;
		}
	}
}

void FlexFXAudioProcessorEditor::FXChainManager::paint(juce::Graphics& g)
{

	g.setColour(getColor(BDSP_COLOR_DARK));
	g.fillPath(bdsp::getRoundedRectangleFromCurveBools(getLocalBounds().toFloat(), corners, universals->roundedRectangleCurve));


	g.setColour(getColor(BDSP_COLOR_PURE_BLACK));
	g.fillRect(cutoutRect);

	bdsp::RearrangeableTabsComponent::paint(g);
}

FlexFXAudioProcessorEditor::FXSlot* FlexFXAudioProcessorEditor::FXChainManager::getComp(int i)
{
	return compCasts[i];
}





void FlexFXAudioProcessorEditor::FXChainManager::addComponent(int i)
{
	bdsp::RearrangeableTabsComponent::addComponent(new FXSlot(p, i, chainBypassButtons[i / numPerTab]), i / numPerTab);

	compCasts.add(dynamic_cast<FXSlot*>(rearrangeComp->comps.getLast()));

}




