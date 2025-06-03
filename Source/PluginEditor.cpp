

#include "PluginEditor.h"


constexpr auto TopBarH = 0.08;
constexpr auto MacroH = 0.125;
constexpr auto HintH = 0.05;
constexpr auto FXW = 0.5;


constexpr auto aspecRatioClosed = 1.618;
constexpr auto aspectRatioOpen = 2.318;


//==============================================================================
FlexFXAudioProcessorEditor::FlexFXAudioProcessorEditor(FlexFXAudioProcessor& p)
	: bdsp::Editor(p)
{

	FXTypeNames = &p.FXTypeNames;




	setBufferedToImage(true);


	init();

	titleBar = std::make_unique<bdsp::Component>(&GUIUniversals);

	titleBar->addAndMakeVisible(topLevelComp->preset->titleBar);
	titleBar->addAndMakeVisible(topLevelComp->undoRedo.get());
	titleBar->addAndMakeVisible(topLevelComp->settingsButton);




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





	topLevelComp->Alert->setRelativeSize(0.618f, 0.618f);
	topLevelComp->Alert->setFonts(2, 1);


	topLevelComp->aspectRatio = aspecRatioClosed;

	topLevelComp->sidebarAspectRatio = aspectRatioOpen;


	modulationTabs = std::make_unique<bdsp::TabsComponent>(&GUIUniversals, juce::StringArray({ "LFOs","ENVs","SEQs" }));
	modulationTabs->setVertical(false);
	modulationTabs->setColors(BDSP_COLOR_BLACK, BDSP_COLOR_WHITE, BDSP_COLOR_DARK);
	modulationTabs->setTabRatio(0.1);
	topLevelComp->addAndMakeVisible(modulationTabs.get());


	modulationTabs->getPage(0)->addAndMakeVisible(LFOs);
	modulationTabs->getPage(1)->addAndMakeVisible(EnvelopeFollowers);
	modulationTabs->getPage(2)->addAndMakeVisible(Sequencers);


	topLevelComp->addComponent(Macros);


	LFOs->setColors(BDSP_COLOR_BLACK);
	EnvelopeFollowers->setColors(BDSP_COLOR_BLACK);
	Sequencers->setColors(BDSP_COLOR_BLACK);

	Macros->setColors(bdsp::NamedColorsIdentifier());
	Macros->shouldDrawDivider = false;

	Macros->setTitleFontIndex(1);
	LFOs->setTitleFontIndex(1);
	EnvelopeFollowers->setTitleFontIndex(1);
	Sequencers->setTitleFontIndex(1);


	LFOs->setVertical(true);
	EnvelopeFollowers->setVertical(true);

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


	topLevelComp->addComponent(fxSlotManager.get());

	title = std::make_unique<AnimatedTitle>(this);


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
		//bdsp::drawDivider(g, juce::Line<float>(macroArea.getRelativePoint(0.0, 0.0), macroArea.getRelativePoint(0.0, 1.0)), GUIUniversals.colors.getColor(BDSP_COLOR_MID), GUIUniversals.dividerSize);



		//g.setColour(GUIUniversals.colors.getColor(BDSP_COLOR_PURE_BLACK));
		//g.drawRoundedRectangle(topBarArea.reduced(GUIUniversals.dividerSize), GUIUniversals.roundedRectangleCurve, GUIUniversals.dividerSize);


		//g.setColour(GUIUniversals.colors.getColor(BDSP_COLOR_MID));
		//g.drawRoundedRectangle(topBarArea.expanded(GUIUniversals.dividerSize / 3), GUIUniversals.roundedRectangleCurve, GUIUniversals.dividerSize / 3);

		g.setColour(GUIUniversals.colors.getColor(BDSP_COLOR_PURE_BLACK));
		auto x = fxSlotManager->dragBoxes.getFirst()->getBoundsRelativeToDesktopManager().getRight() + GUIUniversals.rectThicc;
		bdsp::drawDivider(g, juce::Line<float>(x, FXArea.getY() + GUIUniversals.tabBorderSize, x, FXArea.getBottom() - GUIUniversals.tabBorderSize), GUIUniversals.colors.getColor(BDSP_COLOR_MID), GUIUniversals.dividerSize);
		//g.drawVerticalLine(x, FXArea.getY(), FXArea.getBottom());
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

	bdsp::drawOutlinedRoundedRectangle(g, topBarArea, bdsp::CornerCurves::all, GUIUniversals.roundedRectangleCurve, GUIUniversals.tabBorderSize, GUIUniversals.colors.getColor(BDSP_COLOR_DARK), GUIUniversals.colors.getColor(BDSP_COLOR_BLACK));


	bdsp::drawOutlinedRoundedRectangle(g, macroArea, bdsp::CornerCurves::all, GUIUniversals.roundedRectangleCurve, GUIUniversals.tabBorderSize, GUIUniversals.colors.getColor(BDSP_COLOR_BLACK), GUIUniversals.colors.getColor(BDSP_COLOR_DARK));

	bdsp::drawOutlinedRoundedRectangle(g, FXArea, bdsp::CornerCurves::all, GUIUniversals.roundedRectangleCurve, GUIUniversals.tabBorderSize, GUIUniversals.colors.getColor(BDSP_COLOR_DARK), GUIUniversals.colors.getColor(BDSP_COLOR_BLACK));
	if (getTopLevelGUIComponent()->sidebarOpen)
	{
		bdsp::drawOutlinedRoundedRectangle(g, getTopLevelGUIComponent()->sideBarContainer->getBounds().toFloat(), bdsp::CornerCurves::all, GUIUniversals.roundedRectangleCurve, GUIUniversals.tabBorderSize, GUIUniversals.colors.getColor(BDSP_COLOR_DARK), GUIUniversals.colors.getColor(BDSP_COLOR_BLACK));
	}

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
		GUIUniversals.tabBorderSize = GUIUniversals.rectThicc / 3;
		GUIUniversals.dividerSize = GUIUniversals.rectThicc / 6;
		GUIUniversals.dividerBorder = GUIUniversals.rectThicc;
		GUIUniversals.influenceHoverMenuHeight = h / 11.0 * GUIUniversals.systemScaleFactor;

		GUIUniversals.sliderPopupWidth = 2.5 * GUIUniversals.influenceHoverMenuHeight;

		topBarArea = topLevelComp->mainArea.getBounds().toFloat().withHeight(getHeight() * TopBarH).reduced(GUIUniversals.rectThicc);

		FXArea = topLevelComp->mainArea.getBounds().toFloat().getProportion(juce::Rectangle<float>(0, TopBarH, FXW, 1 - HintH - TopBarH)).reduced(GUIUniversals.rectThicc);

		macroArea = topLevelComp->mainArea.getBounds().toFloat().getProportion(juce::Rectangle<float>(FXW, TopBarH, 1 - FXW, MacroH)).reduced(GUIUniversals.rectThicc);

		modulationTabs->setBounds(bdsp::shrinkRectangleToInt(topLevelComp->mainArea.getBounds().toFloat().getProportion(juce::Rectangle<float>(FXW, TopBarH + MacroH, 1 - FXW, (1 - HintH - TopBarH - MacroH))).reduced(GUIUniversals.rectThicc)));

		Macros->setBounds(bdsp::shrinkRectangleToInt(macroArea));

		LFOs->setBounds(bdsp::shrinkRectangleToInt(modulationTabs->getUsableArea().withZeroOrigin().reduced(GUIUniversals.rectThicc)));
		EnvelopeFollowers->setBounds(bdsp::shrinkRectangleToInt(modulationTabs->getUsableArea().withZeroOrigin().reduced(GUIUniversals.rectThicc)));
		Sequencers->setBounds(bdsp::shrinkRectangleToInt(modulationTabs->getUsableArea().withZeroOrigin().reduced(GUIUniversals.rectThicc)));




		hintBarArea = topLevelComp->mainArea.getBounds().toFloat().withTop(FXArea.getBottom()).reduced(GUIUniversals.rectThicc, 0);

		titleBar->setBounds(bdsp::shrinkRectangleToInt(topBarArea));

		topLevelComp->preset->titleBar.setBounds(bdsp::shrinkRectangleToInt(titleBar->getLocalBounds().getProportion(juce::Rectangle<float>(0.2, 0.15, 0.6, 0.7))));
		topLevelComp->undoRedo->setBounds(bdsp::shrinkRectangleToInt(titleBar->getLocalBounds().getProportion(juce::Rectangle<float>(0.875, 0.2, 0.125, 0.6)).reduced(GUIUniversals.rectThicc)));
		topLevelComp->settingsButton.setBounds(bdsp::shrinkRectangleToInt(titleBar->getLocalBounds().getProportion(juce::Rectangle<float>(0.8, 0.2, 0.075, 0.6)).reduced(GUIUniversals.rectThicc)));

		//topLevelComp->logo->setBounds(bdsp::shrinkRectangleToInt(titleBar->getLocalBounds().getProportion(juce::Rectangle<float>(0, 0.15, 0.05, 0.7)).reduced(0, GUIUniversals.rectThicc)));

		title->setBounds(bdsp::shrinkRectangleToInt(titleBar->getLocalBounds().getProportion(juce::Rectangle<float>(0, 0.1, 0.2, 0.8)).reduced(GUIUniversals.rectThicc)));

		//================================================================================================================================================================================================

		for (int i = 0; i < BDSP_NUMBER_OF_MACROS; ++i)
		{
			Macros->getMacro(i)->setBounds(bdsp::shrinkRectangleToInt(Macros->getLocalBounds().getProportion(juce::Rectangle<float>(i * 1.0 / BDSP_NUMBER_OF_MACROS, 0, 1.0 / BDSP_NUMBER_OF_MACROS, 1)).reduced(GUIUniversals.rectThicc)));
		}


		//	routing->setBounds(bdsp::shrinkRectangleToInt(routingArea.reduced(GUIUniversals.rectThicc)));

			//================================================================================================================================================================================================

		fxSlotManager->setBounds(bdsp::shrinkRectangleToInt(FXArea.reduced(GUIUniversals.rectThicc)));



		//================================================================================================================================================================================================		


		//================================================================================================================================================================================================
		GUIUniversals.hintBar->setBounds(bdsp::shrinkRectangleToInt(hintBarArea));

		//================================================================================================================================================================================================
		auto cornerS = 3 * GUIUniversals.rectThicc;

		corner.setBounds(juce::Rectangle<int>().leftTopRightBottom(getWidth() - cornerS, getHeight() - cornerS, getWidth(), getHeight()));

		//================================================================================================================================================================================================
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




	GUIUniversals.colors.setColor(FXTypeNames->operator[](0), juce::Colour(190, 43, 27)); // Distortion
	GUIUniversals.colors.setColor(FXTypeNames->operator[](1), GUIUniversals.colors.getColor(FXTypeNames->operator[](0))); // BitCrush


	GUIUniversals.colors.setColor(FXTypeNames->operator[](2), juce::Colour(190, 146, 27)); // Filter
	GUIUniversals.colors.setColor(FXTypeNames->operator[](3), GUIUniversals.colors.getColor(FXTypeNames->operator[](2))); // EQ

	GUIUniversals.colors.setColor(FXTypeNames->operator[](4), juce::Colour(190, 27, 189)); // Pitch Shift
	GUIUniversals.colors.setColor(FXTypeNames->operator[](5), GUIUniversals.colors.getColor(FXTypeNames->operator[](4))); // RingMod

	GUIUniversals.colors.setColor(FXTypeNames->operator[](6), juce::Colour(29, 181, 204)); // Chours
	GUIUniversals.colors.setColor(FXTypeNames->operator[](7), GUIUniversals.colors.getColor(FXTypeNames->operator[](6))); // Flanger
	GUIUniversals.colors.setColor(FXTypeNames->operator[](8), GUIUniversals.colors.getColor(FXTypeNames->operator[](6))); // Phaser


	GUIUniversals.colors.setColor(FXTypeNames->operator[](9), juce::Colour(19, 178, 84)); // Panner
	GUIUniversals.colors.setColor(FXTypeNames->operator[](10), GUIUniversals.colors.getColor(FXTypeNames->operator[](9))); // Noise




}




//================================================================================================================================================================================================


FlexFXAudioProcessorEditor::FXSlot::FXSlot(FlexFXAudioProcessorEditor* editor, FlexFXAudioProcessor::FXTypes fxType)
	:bdsp::Component(&editor->GUIUniversals),
	bdsp::GUI_Universals::Listener(&editor->GUIUniversals)
{
	p = editor;

	switch (fxType)
	{
	case FlexFXAudioProcessor::Distortion:
		FXSection = std::make_unique<DistortionUI>(p);
		break;
	case FlexFXAudioProcessor::BitCrush:
		FXSection = std::make_unique<BitCrushUI>(p);
		break;
	case FlexFXAudioProcessor::Filter:
		FXSection = std::make_unique<FilterUI>(p);
		break;
	case FlexFXAudioProcessor::ParamEQ:
		FXSection = std::make_unique<EQUI>(p);
		break;
	case FlexFXAudioProcessor::PitchShift:
		FXSection = std::make_unique<PitchShiftUI>(p);
		break;
	case FlexFXAudioProcessor::RingMod:
		FXSection = std::make_unique<RingModUI>(p);
		break;
	case FlexFXAudioProcessor::Chorus:
		FXSection = std::make_unique<ChorusUI>(p);
		break;
	case FlexFXAudioProcessor::Flanger:
		FXSection = std::make_unique<FlangerUI>(p);
		break;
	case FlexFXAudioProcessor::Phaser:
		FXSection = std::make_unique<PhaserUI>(p);
		break;
	case FlexFXAudioProcessor::Panner:
		FXSection = std::make_unique<PannerUI>(p);
		break;
	case FlexFXAudioProcessor::Noise:
		FXSection = std::make_unique<NoiseUI>(p);
		break;
	default:
		break;
	}
	slotIndex = fxType;


	color = bdsp::NamedColorsIdentifier(p->FXTypeNames->operator[](fxType));


	bypassButton = &FXSection->getBypassButton();


	addAndMakeVisible(FXSection.get());
}


void FlexFXAudioProcessorEditor::FXSlot::paint(juce::Graphics& g)
{

	//if (bypassButton != nullptr)
	//{
	//	auto dividerY = ((*bypassButton)->getBottom() + FXSection->getY()) / 2.0;
	//	g.setColour(getColor(BDSP_COLOR_BLACK));
	//	g.drawHorizontalLine(dividerY, border, getWidth() - border);
	//}

}


void FlexFXAudioProcessorEditor::FXSlot::resized()
{
	border = universals->rectThicc;

	FXSection->setBounds(getLocalBounds());


}








int FlexFXAudioProcessorEditor::FXSlot::getSlotIndex() const
{
	return slotIndex;
}

void FlexFXAudioProcessorEditor::FXSlot::setSlotIndex(int newIdx)
{
	slotIndex = newIdx;
}


void FlexFXAudioProcessorEditor::FXSlot::GUI_UniversalsChanged()
{
	repaint();
}


juce::Rectangle<float> FlexFXAudioProcessorEditor::FXSlot::getDraghandleRect()
{
	return juce::Rectangle<float>(2 * (*bypassButton)->getHeight(), (*bypassButton)->getHeight()).withCentre(juce::Point<float>(getWidth() / 2.0f, (*bypassButton)->getBounds().getCentreY()));
}

FlexFXAudioProcessorEditor::FXUI* FlexFXAudioProcessorEditor::FXSlot::getFxSection()
{
	return FXSection.get();
}





//================================================================================================================================================================================================


FlexFXAudioProcessorEditor::AnimatedTitle::AnimatedTitle(FlexFXAudioProcessorEditor* parent)
	:bdsp::OpenGLComponent(&parent->GUIUniversals)
{
	e = parent;
	e->titleBar->addAndMakeVisible(this);

	glDepthOrder = bdsp::OpenGLComponent::RenderPosition::Front;

	background = BDSP_COLOR_DARK;
	vertexBuffer.resize(4);
	indexBuffer.addArray({
		0,1,2,
		1,2,3
		});

	juce::Array<juce::Colour> colors;
	for (int i = 0; i < e->FXTypeNames->size(); ++i)
	{
		const auto& c = getColor(e->FXTypeNames->operator[](i));
		colors.addIfNotAlreadyThere(c);
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
	:bdsp::RearrangableComponentManagerBase(&editor->GUIUniversals),
	vp(&editor->GUIUniversals),
	vpComp(&editor->GUIUniversals)
{
	p = editor;
	orderState = dynamic_cast<FlexFXAudioProcessor*>(&editor->processor)->fxOrderParam.get();

	//instantSwap = false;
	auto proc = dynamic_cast<FlexFXAudioProcessor*>(&p->audioProcessor);

	horizontal = false;

	for (int i = 0; i < p->FXTypeNames->size(); ++i)
	{
		addComponent(new FXSlot(p, FlexFXAudioProcessor::FXTypes(i)));
		fxSlots.add(dynamic_cast<FXSlot*>(comps.getLast()));
		dragBoxes.getLast()->addAndMakeVisible(fxSlots.getLast()->getFxSection()->getBypassButton().get());
		vpComp.addChildComponent(comps.getLast());

		auto prevFunc = fxSlots[i]->getFxSection()->getBypassButton()->onClick;
		fxSlots[i]->getFxSection()->getBypassButton()->onClick = [=]()
		{
			prevFunc();

			fxSlots[i]->setVisible(!fxSlots[i]->getFxSection()->getBypassButton()->getToggleState());

			resized();
			componentMovedOrResized(vpComp, true, true);
		};


		setDragColor(i, p->FXTypeNames->operator[](i));
		setDragText(i, p->FXTypeNames->operator[](i));


		dragBoxes[i]->dragger.setFontIndex(0);
		dragBoxes[i]->setColors(BDSP_COLOR_BLACK);

	}
	for (int i = 0; i < p->FXTypeNames->size(); ++i)
	{
		fxSlots[i]->getFxSection()->getBypassButton()->onClick();
	}

	onComponentOrderChanged = [=](int indexMoved, int indexMovedTo)
	{
		//orderState->moveSingleItem(indexMoved, indexMovedTo);
		vp.setViewPosition(0, fxSlots[orderState->getOrder()[indexMovedTo]]->getY());
	};

	attach(orderState);

	//dragBoxPaintFunc = [=](juce::Graphics& g, DragBox* db)
	//{
	//	bdsp::drawOutlinedRoundedRectangle(g, db->getLocalBounds().toFloat(), bdsp::CornerCurves::all, universals->roundedRectangleCurve, universals->tabBorderSize, getColor(BDSP_COLOR_BLACK), getColor(BDSP_COLOR_MID));
	//};


	vp.setScrollBarsShown(true, false);
	vp.addAndMakeVisible(vpComp);
	vp.setViewedComponent(&vpComp);

	vpComp.addComponentListener(this);

	vp.setScrollColor(BDSP_COLOR_LIGHT);

	addAndMakeVisible(vp);



}


void FlexFXAudioProcessorEditor::FXChainManager::resized()
{
	if (!fxSlots.isEmpty())
	{


		bool wasResized = vp.getBounds() != getLocalBounds();

		vp.setScrollDistance(getHeight() / 10);

		float borderRatio = 0.01;

		vp.setScrollBarThickness(1.5 * universals->rectThicc);

		auto mainBounds = getLocalBounds().toFloat().withTrimmedRight(vp.getScrollBarThickness()).withTrimmedLeft(getWidth() * 0.25);
		juce::Rectangle<int> bounds;

		dragHandleBounds = getLocalBounds().toFloat().withRight(mainBounds.getX() - universals->rectThicc);
		float currentY = 0;

		auto fxOrder = orderState->getOrder();
		for (int i = 0; i < comps.size(); ++i)
		{
			auto currentFX = fxSlots[fxOrder[i]];
			if (currentFX->isVisible())
			{
				auto h = mainBounds.getWidth() / currentFX->getFxSection()->getDesiredAspectRatio();
				bounds = bdsp::shrinkRectangleToInt(juce::Rectangle<float>(mainBounds.getX(), currentY, mainBounds.getWidth(), h).reduced(universals->rectThicc));
				currentFX->setBounds(bounds);
				currentY += h;
			}

			draggerSlots[i]->setBounds(bdsp::shrinkRectangleToInt(dragHandleBounds.getProportion(juce::Rectangle<float>(0, (float)i / comps.size(), 1, 1.0 / comps.size()))).reduced(0, universals->rectThicc));
			fxSlots[i]->getFxSection()->getBypassButton()->setBounds(juce::Rectangle<int>(0, 0, draggerSlots[i]->getWidth() * 0.2, draggerSlots[i]->getHeight()).reduced(universals->rectThicc / 2));
			dragBoxes[i]->dragger.setBounds(draggerSlots[i]->getLocalBounds().reduced(universals->rectThicc / 2).withLeft(fxSlots[i]->getFxSection()->getBypassButton()->getRight() + 2 * universals->rectThicc));
			fxSlots[i]->getFxSection()->getBypassButton()->setCornerRadius(universals->roundedRectangleCurve - universals->rectThicc / 2);
		}

		for (auto* b : dragBoxes)
		{
			int idx = orderedDragBoxes.indexOf(b);
			b->setBounds(draggerSlots[idx]->getBounds());
		}

		if (!getBounds().isEmpty() && wasResized)
		{
			auto vpPos = (float)vp.getViewPositionY() / vpComp.getHeight();
			vp.setViewPosition(0, vpPos * vpComp.getHeight());
		}
		vpComp.setSize(getWidth(), currentY + universals->rectThicc);

		vp.setBounds(getLocalBounds());
	}
}

void FlexFXAudioProcessorEditor::FXChainManager::paint(juce::Graphics& g)
{
	g.setColour(getColor(BDSP_COLOR_DARK));
	g.fillRoundedRectangle(getLocalBounds().toFloat(), universals->roundedRectangleCurve);

}











