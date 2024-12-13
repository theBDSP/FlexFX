//#pragma once
//
//#include "PluginProcessor.h"
//
//
//
//
//#define routingVisNumPoints 50
//#define routingNumFrames 120
//
//class RoutingSelector : public bdsp::Component, public bdsp::ChoiceComponentCore
//{
//public:
//
//	RoutingSelector(juce::AudioParameterChoice* param, bdsp::GUI_Universals* universalsToUse);
//	virtual ~RoutingSelector() = default;
//
//
//	void resized() override;
//	void paint(juce::Graphics& g) override;
//
//	void setColor(int slotNum, const bdsp::NamedColorsIdentifier& newColor);
//
//	void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& mw) override;
//
//private:
//
//	class RoutingOptionBase : public bdsp::OpenGLLineRenderer, public bdsp::GUI_Universals::Listener
//	{
//	public:
//		RoutingOptionBase(RoutingSelector* parent, int idx);
//		virtual ~RoutingOptionBase() = default;
//
//		void mouseDown(const juce::MouseEvent& e) override;
//		void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& mw) override;
//
//
//		void setColor(int slotNum, const bdsp::NamedColorsIdentifier& newColor);
//
//		void resized() override;
//
//		void paintOverChildren(juce::Graphics& g) override;
//
//		void generateVertexBuffer() override;
//
//
//	protected:
//		RoutingSelector* p = nullptr;
//		juce::OwnedArray<bdsp::OpenGLColor> colors;
//		juce::Array<juce::Rectangle<float>> rects;
//		juce::OwnedArray<juce::Path> letters;
//
//		int index;
//		float hSize = 0.2;
//		float letterScale = 0.8;
//		float lineH = 0.0175;
//
//		unsigned int frameCounter = 0;
//
//		// Inherited via Listener
//		void GUI_UniversalsChanged() override;
//	};
//
//	class RoutingOption4 : public RoutingOptionBase
//	{
//	public:
//
//		RoutingOption4(RoutingSelector* parent);
//
//
//		void resized() override;
//
//	};
//	class RoutingOption3 : public RoutingOptionBase
//	{
//	public:
//
//		RoutingOption3(RoutingSelector* parent);
//
//
//		void resized() override;
//
//	};
//
//	class RoutingOption2 : public RoutingOptionBase
//	{
//	public:
//
//		RoutingOption2(RoutingSelector* parent);
//
//
//		void resized() override;
//
//	};
//	class RoutingOption1 : public RoutingOptionBase
//	{
//	public:
//
//		RoutingOption1(RoutingSelector* parent);
//
//
//		void resized() override;
//
//	};
//	class RoutingOption0 : public RoutingOptionBase
//	{
//	public:
//
//		RoutingOption0(RoutingSelector* parent);
//
//
//		void resized() override;
//
//	};
//
//
//	juce::OwnedArray<RoutingOptionBase> options;
//
//	// Inherited via ChoiceComponentCore
//	void parameterChanged(int idx) override;
//};