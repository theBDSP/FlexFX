//#include "RoutingSelector.h"
//
//RoutingSelector::RoutingSelector(juce::AudioParameterChoice* param, bdsp::GUI_Universals* universalsToUse)
//	:bdsp::Component(universalsToUse),
//	bdsp::ChoiceComponentCore(param, this)
//{
//	options.add(new RoutingOption0(this));
//	addAndMakeVisible(options.getLast());
//	options.add(new RoutingOption1(this));
//	addAndMakeVisible(options.getLast());
//	options.add(new RoutingOption2(this));
//	addAndMakeVisible(options.getLast());
//	options.add(new RoutingOption3(this));
//	addAndMakeVisible(options.getLast());
//	options.add(new RoutingOption4(this));
//	addAndMakeVisible(options.getLast());
//
//	attach(param, universals->undoManager);
//
//}
//
//void RoutingSelector::resized()
//{
//	//auto b = getWidth() * 0.025;
//	//auto w = 1.0 / 3.0 * getWidth() - 2 * b;
//	//auto h = getHeight() / 2.0 - 2 * b;
//
//
//	//auto rect = juce::Rectangle<float>(w, h);
//
//	//options[0]->setBounds(bdsp::shrinkRectangleToInt(rect.withRightX(getWidth() / 2.0 - b).withCentreY(getHeight() / 4.0)));
//	//options[1]->setBounds(bdsp::shrinkRectangleToInt(rect.withX(getWidth() / 2.0 + b).withCentreY(getHeight() / 4.0)));
//
//	//options[2]->setBounds(bdsp::shrinkRectangleToInt(rect.withX(b).withCentreY(3 * getHeight() / 4.0)));
//	//options[3]->setBounds(bdsp::shrinkRectangleToInt(rect.withCentreX(getWidth() / 2.0).withCentreY(3 * getHeight() / 4.0)));
//	//options[4]->setBounds(bdsp::shrinkRectangleToInt(rect.withRightX(getWidth() - b).withCentreY(3 * getHeight() / 4.0)));
//
//
//	auto b = 0.025;
//	auto w = (1 - 4 * b) / 5;
//
//	for (int i = 0; i < options.size(); ++i)
//	{
//		options[i]->setBounds(getLocalBounds().getProportion(juce::Rectangle<float>(i * (b + w), b, w, 1 - 2 * b)).reduced(universals->roundedRectangleCurve));
//	}
//
//}
//
//void RoutingSelector::paint(juce::Graphics& g)
//{
//
//	for (auto o : options)
//	{
//		g.setColour(o->getBackgroundColor());
//		g.fillRoundedRectangle(o->getBounds().toFloat().expanded(universals->roundedRectangleCurve), universals->roundedRectangleCurve);
//	}
//}
//
//void RoutingSelector::setColor(int slotNum, const bdsp::NamedColorsIdentifier& newColor)
//{
//	for (auto o : options)
//	{
//		o->setColor(slotNum, newColor);
//	}
//}
//
//void RoutingSelector::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& mw)
//{
//	scrollItems(mw.isReversed ? mw.deltaY > 0 : mw.deltaY < 0);
//}
//
//// Inherited via ChoiceComponentCore
//
//void RoutingSelector::parameterChanged(int idx)
//{
//	for (int i = 0; i < options.size(); ++i)
//	{
//		options[i]->setEnabled(i == idx);
//	}
//	repaint();
//}
//
////================================================================================================================================================================================================
//
//
//
//RoutingSelector::RoutingOptionBase::RoutingOptionBase(RoutingSelector* parent, int idx)
//	:bdsp::OpenGLLineRenderer(parent->universals, { routingVisNumPoints ,routingVisNumPoints ,routingVisNumPoints ,routingVisNumPoints ,routingVisNumPoints ,routingVisNumPoints ,routingVisNumPoints ,routingVisNumPoints }),
//	bdsp::GUI_Universals::Listener(parent->universals)
//{
//
//	p = parent;
//	index = idx;
//
//	setBackgroundColor(BDSP_COLOR_PURE_BLACK, BDSP_COLOR_DARK);
//
//	rects.resize(numFXSlots);
//
//	colors.add(new bdsp::OpenGLColor(universals, extendedComponent));
//	for (int i = 0; i < numFXSlots; ++i)
//	{
//		colors.add(new bdsp::OpenGLColor(universals, extendedComponent));
//		letters.add(new juce::Path());
//	}
//
//
//	colors.getLast()->setColors(BDSP_COLOR_WHITE, bdsp::NamedColorsIdentifier(BDSP_COLOR_WHITE).mixedWith(background.getColorID(false), 1 - universals->disabledAlpha));
//	for (int i = 0; i < linePoints.size(); ++i)
//	{
//		linePoints[i]->resizeByVertex(routingVisNumPoints);
//	}
//
//	setHasAlpha(true);
//
//	GUI_UniversalsChanged();
//
//}
//
//
//void RoutingSelector::RoutingOptionBase::mouseDown(const juce::MouseEvent& e)
//{
//	p->selectItem(index);
//}
//
//void RoutingSelector::RoutingOptionBase::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& mw)
//{
//	p->mouseWheelMove(e, mw);
//}
//
//void RoutingSelector::RoutingOptionBase::setColor(int slotNum, const bdsp::NamedColorsIdentifier& newColor)
//{
//	colors[slotNum]->setColors(newColor, newColor.mixedWith(background.getColorID(false), 1 - universals->disabledAlpha));
//	repaint();
//}
//
//void RoutingSelector::RoutingOptionBase::resized()
//{
//	//hSize = hSize * getWidth() / float(getHeight());
//	auto b = (1 - letterScale) / 2;
//	for (int i = 0; i < rects.size(); ++i)
//	{
//		bdsp::scaleToFit(*letters[i], bdsp::confineToAspectRatio(rects[i].getProportion(juce::Rectangle<float>(b, b, letterScale, letterScale)), 1));
//	}
//	for (int i = 0; i < linePoints.size(); ++i)
//	{
//		lineScreenThickness.set(i, getHeight() * lineH);
//	}
//
//	bdsp::OpenGLLineRenderer::resized();
//
//}
//
//void RoutingSelector::RoutingOptionBase::paintOverChildren(juce::Graphics& g)
//{
//	auto alpha = isEnabled() ? 1.0f : universals->disabledAlpha;
//
//
//
//	for (int i = 0; i < letters.size(); ++i)
//	{
//
//		g.setColour(getColor(BDSP_COLOR_MID).withAlpha(alpha));
//		//g.fillRoundedRectangle(rects[i], universals->roundedRectangleCurve);
//
//		g.setColour(colors[i]->getColor(isEnabled()));
//		g.fillPath(*letters[i]);
//	}
//
//
//}
//
//void RoutingSelector::RoutingOptionBase::generateVertexBuffer()
//{
//	float r, g, b, a;
//
//	for (int i = 0; i < lineVertexBuffer.size(); ++i) // any unneccesary lines should be removed from array
//	{
//		if (i >= 4)// inputlines
//		{
//			colors[4]->getComponents(r, g, b, a);
//		}
//		else
//		{
//			colors[i]->getComponents(r, g, b, a);
//		}
//		float frameProp = (frameCounter % (routingNumFrames - 1)) / (routingNumFrames - 1.0f);
//		for (int j = 0; j < linePoints[i]->getNumVerticies(); ++j)
//		{
//			linePoints[i]->set(j, { linePoints[i]->getAttribute(j, 0), linePoints[i]->getAttribute(j, 1), r, g, b, abs(1 - 2 * fmodf(1.0f + frameProp - (linePoints[i]->getAttribute(j, 0) + 1) / 2,1.0f)) });
//		}
//	}
//	frameCounte++r;
//	generateLineTriangles();
//}
//
//// Inherited via Listener
//
//void RoutingSelector::RoutingOptionBase::GUI_UniversalsChanged()
//{
//	letters[0]->clear();
//	juce::GlyphArrangement A;
//	A.addLineOfText(universals->Fonts[2].font, "A", 0, 0);
//	A.createPath(*letters[0]);
//
//	letters[1]->clear();
//	juce::GlyphArrangement B;
//	B.addLineOfText(universals->Fonts[2].font, "B", 0, 0);
//	B.createPath(*letters[1]);
//
//	letters[2]->clear();
//	juce::GlyphArrangement C;
//	C.addLineOfText(universals->Fonts[2].font, "C", 0, 0);
//	C.createPath(*letters[2]);
//
//	letters[3]->clear();
//	juce::GlyphArrangement D;
//	D.addLineOfText(universals->Fonts[2].font, "D", 0, 0);
//	D.createPath(*letters[3]);
//}
////================================================================================================================================================================================================
//
//RoutingSelector::RoutingOption4::RoutingOption4(RoutingSelector* parent)
//	:RoutingOptionBase(parent, 4)
//{
//	auto yb = (2.0f - 8 * hSize) / 5.0f;
//
//	auto y0 = 1.0f - yb - hSize;
//	auto y1 = y0 - yb - 2 * hSize;
//	auto y2 = y1 - yb - 2 * hSize;
//	auto y3 = y2 - yb - 2 * hSize;
//	for (int i = 0; i < routingVisNumPoints; ++i)
//	{
//
//		linePoints[4]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),-1.0f,-hSize),y0,0,0,0,0 });
//		linePoints[0]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),hSize,1.0f),y0,0,0,0,0 });
//
//
//		linePoints[5]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),-1.0f,-hSize),y1,0,0,0,0 });
//		linePoints[1]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),hSize,1.0f),y1,0,0,0,0 });
//
//		linePoints[6]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),-1.0f,-hSize),y2,0,0,0,0 });
//		linePoints[2]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),hSize,1.0f),y2,0,0,0,0 });
//
//
//		linePoints[7]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),-1.0f,-hSize),y3,0,0,0,0 });
//		linePoints[3]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),hSize,1.0f),y3,0,0,0,0 });
//
//	}
//
//}
//
//void RoutingSelector::RoutingOption4::resized()
//{
//	auto yb = (1.0f - 4 * hSize) / 5.0f;
//
//	for (int i = 0; i < rects.size(); ++i)
//	{
//		rects.set(i, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(0.5 - hSize / 2, (i + 1) * yb + i * hSize, hSize, hSize)));
//	}
//
//
//	RoutingOptionBase::resized();
//}
//
//RoutingSelector::RoutingOption3::RoutingOption3(RoutingSelector* parent)
//	:RoutingOptionBase(parent, 3)
//{
//
//	linePoints.removeLast();
//	lineIndexBuffer.removeLast();
//	lineVertexBuffer.removeLast();
//
//	numLines = 7;
//
//	auto yb = (2.0f - 6 * hSize) / 4.0f;
//
//	auto y0 = 1.0f - yb - hSize;
//	auto y1 = y0 - yb - 2 * hSize;
//	auto y2 = y1 - yb - 2 * hSize;
//
//	auto xb = (2.0f - 4 * hSize) / 3.0f;
//	auto x0 = -1.0f;
//	auto x1 = x0 + xb;
//	auto x2 = x1 + 2 * hSize;
//	auto x3 = x2 + xb;
//	auto x4 = x3 + 2 * hSize;
//	auto x5 = 1.0f;
//
//	for (int i = 0; i < routingVisNumPoints; ++i)
//	{
//		linePoints[4]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x0,x1),y0,0,0,0,0 });
//		linePoints[0]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x2,x3),y0,0,0,0,0 });
//		linePoints[1]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x4,x5),y0,0,0,0,0 });
//
//		linePoints[5]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x0,x1),y1,0,0,0,0 });
//		linePoints[2]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x2,x5),y1,0,0,0,0 });
//
//
//
//		linePoints[6]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x0,x1),y2,0,0,0,0 });
//		linePoints[3]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x2,x5),y2,0,0,0,0 });
//
//
//
//
//
//
//	}
//}
//
//void RoutingSelector::RoutingOption3::resized()
//{
//	auto yb = (1.0f - 3 * hSize) / 4.0f;
//	auto xb = (1.0f - 2 * hSize) / 3.0f;
//
//	rects.set(0, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(xb, yb, hSize, hSize)));
//	rects.set(1, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(2 * xb + hSize, yb, hSize, hSize)));
//	rects.set(2, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(xb, 2 * yb + hSize, hSize, hSize)));
//	rects.set(3, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(xb, 3 * yb + 2 * hSize, hSize, hSize)));
//
//
//
//
//	RoutingOptionBase::resized();
//}
//
//
//RoutingSelector::RoutingOption2::RoutingOption2(RoutingSelector* parent)
//	:RoutingOptionBase(parent, 2)
//{
//
//	linePoints.removeLast();
//	lineIndexBuffer.removeLast();
//	lineVertexBuffer.removeLast();
//	linePoints.removeLast();
//	lineIndexBuffer.removeLast();
//	lineVertexBuffer.removeLast();
//
//	numLines = 6;
//
//	auto yb = (2.0f - 4 * hSize) / 3.0f;
//
//	auto y0 = 1.0f - yb - hSize;
//	auto y1 = y0 - yb - 2 * hSize;
//
//	auto xb = (2.0f - 4 * hSize) / 3.0f;
//	auto x0 = -1.0f;
//	auto x1 = x0 + xb;
//	auto x2 = x1 + 2 * hSize;
//	auto x3 = x2 + xb;
//	auto x4 = x3 + 2 * hSize;
//	auto x5 = 1.0f;
//
//	for (int i = 0; i < routingVisNumPoints; ++i)
//	{
//		linePoints[4]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x0,x1),y0,0,0,0,0 });
//		linePoints[0]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x2,x3),y0,0,0,0,0 });
//		linePoints[1]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x4,x5),y0,0,0,0,0 });
//
//
//		linePoints[5]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x0,x1),y1,0,0,0,0 });
//		linePoints[2]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x2,x3),y1,0,0,0,0 });
//		linePoints[3]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x4,x5),y1,0,0,0,0 });
//	}
//}
//
//void RoutingSelector::RoutingOption2::resized()
//{
//	auto yb = (1.0f - 2 * hSize) / 3.0f;
//	auto xb = (1.0f - 2 * hSize) / 3.0f;
//
//	rects.set(0, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(xb, yb, hSize, hSize)));
//	rects.set(1, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(2 * xb + hSize, yb, hSize, hSize)));
//	rects.set(2, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(xb, 2 * yb + hSize, hSize, hSize)));
//	rects.set(3, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(2 * xb + hSize, 2 * yb + hSize, hSize, hSize)));
//
//
//
//
//
//	RoutingOptionBase::resized();
//}
//RoutingSelector::RoutingOption1::RoutingOption1(RoutingSelector* parent)
//	:RoutingOptionBase(parent, 1)
//{
//
//	linePoints.removeLast();
//	lineIndexBuffer.removeLast();
//	lineVertexBuffer.removeLast();
//	linePoints.removeLast();
//	lineIndexBuffer.removeLast();
//	lineVertexBuffer.removeLast();
//
//	numLines = 6;
//
//	auto yb = (2.0f - 4 * hSize) / 3.0f;
//
//	auto y0 = 1.0f - yb - hSize;
//	auto y1 = y0 - yb - 2 * hSize;
//
//	auto xb = (2.0f - 6 * hSize) / 4.0f;
//	auto x0 = -1.0f;
//	auto x1 = x0 + xb;
//	auto x2 = x1 + 2 * hSize;
//	auto x3 = x2 + xb;
//	auto x4 = x3 + 2 * hSize;
//	auto x5 = x4 + xb;
//	auto x6 = x5 + 2 * hSize;
//	auto x7 = 1.0f;
//
//	for (int i = 0; i < routingVisNumPoints; ++i)
//	{
//		linePoints[4]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x0,x1),y0,0,0,0,0 });
//		linePoints[0]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x2,x3),y0,0,0,0,0 });
//		linePoints[1]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x4,x5),y0,0,0,0,0 });
//		linePoints[2]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x6,x7),y0,0,0,0,0 });
//
//
//		linePoints[5]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x0,x1),y1,0,0,0,0 });
//		linePoints[3]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x2,x7),y1,0,0,0,0 });
//	}
//}
//
//void RoutingSelector::RoutingOption1::resized()
//{
//	auto yb = (1.0f - 2 * hSize) / 3.0f;
//	auto xb = (1.0f - 3 * hSize) / 4.0f;
//
//	rects.set(0, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(xb, yb, hSize, hSize)));
//	rects.set(1, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(2 * xb + hSize, yb, hSize, hSize)));
//	rects.set(2, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(3 * xb + 2 * hSize, yb, hSize, hSize)));
//
//	rects.set(3, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(xb, 2 * yb + hSize, hSize, hSize)));
//
//
//	RoutingOptionBase::resized();
//}
//RoutingSelector::RoutingOption0::RoutingOption0(RoutingSelector* parent)
//	:RoutingOptionBase(parent, 0)
//{
//
//	linePoints.removeLast();
//	lineIndexBuffer.removeLast();
//	lineVertexBuffer.removeLast();
//	linePoints.removeLast();
//	lineIndexBuffer.removeLast();
//	lineVertexBuffer.removeLast();
//	linePoints.removeLast();
//	lineIndexBuffer.removeLast();
//	lineVertexBuffer.removeLast();
//
//	numLines = 5;
//
//	auto y = 0.0f;
//
//	auto xb = (2.0f - 8 * hSize) / 5.0f;
//	auto x0 = -1.0f;
//	auto x1 = x0 + xb;
//	auto x2 = x1 + 2 * hSize;
//	auto x3 = x2 + xb;
//	auto x4 = x3 + 2 * hSize;
//	auto x5 = x4 + xb;
//	auto x6 = x5 + 2 * hSize;
//	auto x7 = x6 + xb;
//	auto x8 = x7 + 2 * hSize;
//	auto x9 = 1.0f;
//
//	for (int i = 0; i < routingVisNumPoints; ++i)
//	{
//		linePoints[4]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x0,x1),y,0,0,0,0 });
//		linePoints[0]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x2,x3),y,0,0,0,0 });
//		linePoints[1]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x4,x5),y,0,0,0,0 });
//		linePoints[2]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x6,x7),y,0,0,0,0 });
//		linePoints[3]->set(i, { juce::jmap(i / (routingVisNumPoints - 1.0f),x8,x9),y,0,0,0,0 });
//	}
//}
//
//void RoutingSelector::RoutingOption0::resized()
//{
//	auto y = 0.5f - hSize / 2;
//	auto xb = (1.0f - 4 * hSize) / 5.0f;
//
//	rects.set(0, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(xb, y, hSize, hSize)));
//	rects.set(1, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(2 * xb + hSize, y, hSize, hSize)));
//	rects.set(2, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(3 * xb + 2 * hSize, y, hSize, hSize)));
//	rects.set(3, getLocalBounds().toFloat().getProportion(juce::Rectangle<float>(4 * xb + 3 * hSize, y, hSize, hSize)));
//
//
//
//	RoutingOptionBase::resized();
//}
