#pragma once
#ifndef ES_APP_COMPONENTS_TEXT_LIST_COMPONENT_H
#define ES_APP_COMPONENTS_TEXT_LIST_COMPONENT_H

#include "components/IList.h"
#include "math/Misc.h"
#include "utils/StringUtil.h"
#include "Log.h"
#include "Sound.h"
#include <memory>
#include "components/ScrollbarComponent.h"
#include "Settings.h"

class TextCache;

struct TextListData
{
	unsigned int colorId;
	std::shared_ptr<TextCache> textCache;
};

//A graphical list. Supports multiple colors for rows and scrolling.
template <typename T>
class TextListComponent : public IList<TextListData, T>
{
protected:
	using IList<TextListData, T>::mEntries;
	using IList<TextListData, T>::listUpdate;
	using IList<TextListData, T>::listInput;
	using IList<TextListData, T>::listRenderTitleOverlay;
	using IList<TextListData, T>::getTransform;
	using IList<TextListData, T>::mSize;
	using IList<TextListData, T>::mCursor;
	using IList<TextListData, T>::Entry;

public:
	using IList<TextListData, T>::size;
	using IList<TextListData, T>::isScrolling;
	using IList<TextListData, T>::stopScrolling;

	TextListComponent(Window* window);
	
	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Transform4x4f& parentTrans) override;
	void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties) override;

	void add(const std::string& name, const T& obj, unsigned int colorId);
	
	enum Alignment
	{
		ALIGN_LEFT,
		ALIGN_CENTER,
		ALIGN_RIGHT
	};

	inline void setAlignment(Alignment align) { mAlignment = align; }

	inline void setCursorChangedCallback(const std::function<void(CursorState state)>& func) { mCursorChangedCallback = func; }

	inline void setFont(const std::shared_ptr<Font>& font)
	{
		mFont = font;
		for(auto it = mEntries.begin(); it != mEntries.end(); it++)
			it->data.textCache.reset();
	}

	inline void setUppercase(bool uppercase) 
	{
		mUppercase = uppercase;
		for(auto it = mEntries.begin(); it != mEntries.end(); it++)
			it->data.textCache.reset();
	}

	inline void setSelectorHeight(float selectorScale) { mSelectorHeight = selectorScale; }
	inline void setSelectorOffsetY(float selectorOffsetY) { mSelectorOffsetY = selectorOffsetY; }
	inline void setSelectorColor(unsigned int color) { mSelectorColor = color; }
	inline void setSelectorColorEnd(unsigned int color) { mSelectorColorEnd = color; }
	inline void setSelectorColorGradientHorizontal(bool horizontal) { mSelectorColorGradientHorizontal = horizontal; }
	inline void setSelectedColor(unsigned int color) { mSelectedColor = color; }
	inline void setColor(unsigned int id, unsigned int color) { mColors[id] = color; }
	inline void setLineSpacing(float lineSpacing) { mLineSpacing = lineSpacing; }

	virtual void onShow() override;

	void resetLastCursor() { mLastCursor = -1; }
	int getLastCursor() { return mLastCursor; }

protected:
	virtual void onScroll(int /*amt*/) { if(!mScrollSound.empty()) Sound::get(mScrollSound)->play(); }
	virtual void onCursorChanged(const CursorState& state);

private:
	int mMarqueeOffset;
	int mMarqueeOffset2;
	int mMarqueeTime;

	int mLineCount;

	int mLastCursor;
	CursorState mLastCursorState;

	Alignment mAlignment;
	float mHorizontalMargin;

	std::function<void(CursorState state)> mCursorChangedCallback;

	std::shared_ptr<Font> mFont;
	bool mUppercase;
	float mLineSpacing;
	float mSelectorHeight;
	float mSelectorOffsetY;
	unsigned int mSelectorColor;
	unsigned int mSelectorColorEnd;
	bool mSelectorColorGradientHorizontal = true;
	unsigned int mSelectedColor;
	std::string mScrollSound;
	static const unsigned int COLOR_ID_COUNT = 2;
	unsigned int mColors[COLOR_ID_COUNT];

	ImageComponent mSelectorImage;
	
	ScrollbarComponent mScrollbar;

};

template <typename T>
TextListComponent<T>::TextListComponent(Window* window) :
	IList<TextListData, T>(window), mSelectorImage(window), mScrollbar(window)
{
	mLineCount = -1;
	mMarqueeOffset = 0;
	mMarqueeOffset2 = 0;
	mMarqueeTime = 0;
	mLastCursor = -1;
	mLastCursorState = CursorState::CURSOR_STOPPED;

	mHorizontalMargin = 0;
	mAlignment = ALIGN_CENTER;

	mFont = Font::get(FONT_SIZE_MEDIUM);
	mUppercase = false;
	mLineSpacing = 1.5f;
	mSelectorHeight = mFont->getSize() * 1.5f;
	mSelectorOffsetY = 0;
	mSelectorColor = 0x000000FF;
	mSelectorColorEnd = 0x000000FF;
	mSelectorColorGradientHorizontal = true;
	mSelectedColor = 0;
	mColors[0] = 0x0000FFFF;
	mColors[1] = 0x00FF00FF;
}

template <typename T>
void TextListComponent<T>::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();
	
	std::shared_ptr<Font>& font = mFont;

	if(size() == 0)
		return;

	if (!Renderer::isVisibleOnScreen(trans.translation().x(), trans.translation().y(), mSize.x(), mSize.y()))
		return;

	float entrySize = Math::max(font->getHeight(1.0), (float)font->getSize()) * mLineSpacing;
	if (mLineCount > 0)
		entrySize = mSize.y() / mLineCount;

	int startEntry = 0;

	//number of entries that can fit on the screen simultaniously
	int screenCount = mLineCount > 0 ? mLineCount : (int)(mSize.y() / entrySize);
	
	if(size() >= screenCount)
	{
		startEntry = mCursor - screenCount/2;
		if(startEntry < 0)
			startEntry = 0;
		if(startEntry >= size() - screenCount)
			startEntry = size() - screenCount;
	}

	float y = 0;

	int listCutoff = startEntry + screenCount;
	if(listCutoff > size())
		listCutoff = size();

	// draw selector bar
	if(startEntry < listCutoff)
	{
		if (mSelectorImage.hasImage()) 
		{
			mSelectorImage.setPosition(0.f, (mCursor - startEntry)*entrySize + mSelectorOffsetY, 0.f);
			mSelectorImage.render(trans);
		} 
		else 
		{
			Renderer::setMatrix(trans);			
			Renderer::drawRect(0.0f, (mCursor - startEntry)*entrySize + mSelectorOffsetY, mSize.x(),
					mSelectorHeight, mSelectorColor, mSelectorColorEnd, mSelectorColorGradientHorizontal);
		}
	}

	// clip to inside margins
	Vector3f dim(mSize.x(), mSize.y(), 0);
	dim = trans * dim - trans.translation();
	Renderer::pushClipRect(Vector2i((int)(trans.translation().x() + mHorizontalMargin), (int)trans.translation().y()), 
		Vector2i((int)(dim.x() - mHorizontalMargin*2), (int)dim.y()));

	for(int i = startEntry; i < listCutoff; i++)
	{
		typename IList<TextListData, T>::Entry& entry = mEntries.at((unsigned int)i);

		unsigned int color;
		if(mCursor == i && mSelectedColor)
			color = mSelectedColor;
		else
			color = mColors[entry.data.colorId];

		if(!entry.data.textCache)
			entry.data.textCache = std::unique_ptr<TextCache>(font->buildTextCache(mUppercase ? Utils::String::toUpper(entry.name) : entry.name, 0, 0, 0x000000FF));

		entry.data.textCache->setColor(color);

		Vector3f offset(0, y, 0);

		if (mLineCount > 0) // Vertical center
			offset[1] += (int)((entrySize - entry.data.textCache->metrics.size.y()) / 2);

		switch(mAlignment)
		{
		case ALIGN_LEFT:
			offset[0] = mHorizontalMargin;
			break;
		case ALIGN_CENTER:
			offset[0] = (int)((mSize.x() - entry.data.textCache->metrics.size.x()) / 2);
			if(offset[0] < mHorizontalMargin)
				offset[0] = mHorizontalMargin;
			break;
		case ALIGN_RIGHT:
			offset[0] = (mSize.x() - entry.data.textCache->metrics.size.x());
			offset[0] -= mHorizontalMargin;
			if(offset[0] < mHorizontalMargin)
				offset[0] = mHorizontalMargin;
			break;
		}

		// render text
		Transform4x4f drawTrans = trans;

		// currently selected item text might be scrolling
		if((mCursor == i) && (mMarqueeOffset > 0))
			drawTrans.translate(offset - Vector3f((float)mMarqueeOffset, 0, 0));
		else
			drawTrans.translate(offset);

		Renderer::setMatrix(drawTrans);

		if (Settings::DebugText())
		{
			auto sz = mFont->sizeText(mUppercase ? Utils::String::toUpper(entry.name) : entry.name);

			Renderer::popClipRect();
			Renderer::drawRect(0.0f, 0.0f, sz.x(), sz.y(), 0xFF000033, 0xFF000033);
			Renderer::pushClipRect(Vector2i((int)(trans.translation().x() + mHorizontalMargin), (int)trans.translation().y()),
				Vector2i((int)(dim.x() - mHorizontalMargin * 2), (int)dim.y()));
		}

		font->renderTextCache(entry.data.textCache.get());

		// render currently selected item text again if
		// marquee is scrolled far enough for it to repeat
		if((mCursor == i) && (mMarqueeOffset2 < 0))
		{
			drawTrans = trans;
			drawTrans.translate(offset - Vector3f((float)mMarqueeOffset2, 0, 0));
			Renderer::setMatrix(drawTrans);
			font->renderTextCache(entry.data.textCache.get());
		}

		y += entrySize;
	}

	Renderer::popClipRect();

	listRenderTitleOverlay(trans);

	GuiComponent::renderChildren(trans);

	if (mScrollbar.isEnabled())
	{
		mScrollbar.setContainerBounds(GuiComponent::getPosition(), GuiComponent::getSize());
		mScrollbar.setRange(0, entrySize * mEntries.size(), mSize.y());
		mScrollbar.setScrollPosition(startEntry * entrySize);
		mScrollbar.render(parentTrans);
	}
}

template <typename T>
bool TextListComponent<T>::input(InputConfig* config, Input input)
{
	if(size() > 0)
	{
		if(input.value != 0)
		{
			if(config->isMappedLike("down", input))
			{
				listInput(1);
				return true;
			}

			if(config->isMappedLike("up", input))
			{
				listInput(-1);
				return true;
			}
			if(config->isMappedTo("pagedown", input))
			{
				listInput(10);
				return true;
			}

			if(config->isMappedTo("pageup", input))
			{
				listInput(-10);
				return true;
			}
		}else{
			if(config->isMappedLike("down", input) || config->isMappedLike("up", input) || 
				config->isMappedLike("pagedown", input) || config->isMappedLike("pageup", input))
			{
				stopScrolling();
			}
		}
	}

	return GuiComponent::input(config, input);
}

template <typename T>
void TextListComponent<T>::update(int deltaTime)
{
	mScrollbar.update(deltaTime);

	listUpdate(deltaTime);

	if(!isScrolling() && size() > 0)
	{
		// always reset the marquee offsets
		mMarqueeOffset  = 0;
		mMarqueeOffset2 = 0;

		std::string name = mEntries.at((unsigned int)mCursor).name;
		if (mUppercase)
			name = Utils::String::toUpper(name);

		// if we're not scrolling and this object's text goes outside our size, marquee it!
		const float textLength = mFont->sizeText(name).x();
		const float limit      = mSize.x() - mHorizontalMargin * 2;

		if(textLength > limit)
		{
			// loop
			// pixels per second ( based on nes-mini font at 1920x1080 to produce a speed of 200 )
			const float speed        = mFont->sizeText("ABCDEFGHIJKLMNOPQRSTUVWXYZ").x() * 0.247f;
			const float delay        = 3000;
			const float scrollLength = textLength;
			const float returnLength = speed * 1.5f;
			const float scrollTime   = (scrollLength * 1000) / speed;
			const float returnTime   = (returnLength * 1000) / speed;
			const int   maxTime      = (int)(delay + scrollTime + returnTime);

			mMarqueeTime += deltaTime;
			while(mMarqueeTime > maxTime)
				mMarqueeTime -= maxTime;

			mMarqueeOffset = (int)(Math::Scroll::loop(delay, scrollTime + returnTime, (float)mMarqueeTime, scrollLength + returnLength));

			if(mMarqueeOffset > (scrollLength - (limit - returnLength)))
				mMarqueeOffset2 = (int)(mMarqueeOffset - (scrollLength + returnLength));
		}
	}

	GuiComponent::update(deltaTime);
}

//list management stuff
template <typename T>
void TextListComponent<T>::add(const std::string& name, const T& obj, unsigned int color)
{
	assert(color < COLOR_ID_COUNT);

	typename IList<TextListData, T>::Entry entry;
	entry.name = name;
	entry.object = obj;
	entry.data.colorId = color;
	static_cast<IList< TextListData, T >*>(this)->add(entry);
}

template <typename T>
void TextListComponent<T>::onCursorChanged(const CursorState& state)
{
	mMarqueeOffset = 0;
	mMarqueeOffset2 = 0;
	mMarqueeTime = 0;

	mScrollbar.onCursorChanged();

	LOG(LogDebug) << "mCursor \"" << mCursor << "\" state  \"" << state << "\"";

	if (mLastCursor != mCursor || mLastCursorState != state)
	{
		if (mCursorChangedCallback)
			mCursorChangedCallback(state);

		mLastCursor = mCursor;
		mLastCursorState = state;
	}
}

template<typename T>
void TextListComponent<T>::onShow()
{	
	GuiComponent::onShow();

	mMarqueeOffset = 0;
	mMarqueeOffset2 = 0;
	mMarqueeTime = 0;

	mScrollbar.onCursorChanged();
}

template <typename T>
void TextListComponent<T>::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	GuiComponent::applyTheme(theme, view, element, properties);

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "textlist");
	if(!elem)
		return;

	mScrollbar.fromTheme(theme, view, element, "textlist");

	using namespace ThemeFlags;
	if(properties & COLOR)
	{
		if(elem->has("selectorColor"))
		{
			setSelectorColor(elem->get<unsigned int>("selectorColor"));
			setSelectorColorEnd(elem->get<unsigned int>("selectorColor"));
		}
		if (elem->has("selectorColorEnd"))
			setSelectorColorEnd(elem->get<unsigned int>("selectorColorEnd"));
		if (elem->has("selectorGradientType"))
			setSelectorColorGradientHorizontal(elem->get<std::string>("selectorGradientType").compare("horizontal"));
		if(elem->has("selectedColor"))
			setSelectedColor(elem->get<unsigned int>("selectedColor"));
		if(elem->has("primaryColor"))
			setColor(0, elem->get<unsigned int>("primaryColor"));
		if(elem->has("secondaryColor"))
			setColor(1, elem->get<unsigned int>("secondaryColor"));
	}

	setFont(Font::getFromTheme(elem, properties, mFont));
	const float selectorHeight = Math::max(mFont->getHeight(1.0), (float)mFont->getSize()) * mLineSpacing;
	setSelectorHeight(selectorHeight);

	if(properties & SOUND && elem->has("scrollSound"))
		mScrollSound = elem->get<std::string>("scrollSound");

	if(properties & ALIGNMENT)
	{
		if(elem->has("alignment"))
		{
			const std::string& str = elem->get<std::string>("alignment");
			if(str == "left")
				setAlignment(ALIGN_LEFT);
			else if(str == "center")
				setAlignment(ALIGN_CENTER);
			else if(str == "right")
				setAlignment(ALIGN_RIGHT);
			else
				LOG(LogError) << "Unknown TextListComponent alignment \"" << str << "\"!";
		}

		if(elem->has("horizontalMargin"))
			mHorizontalMargin = elem->get<float>("horizontalMargin") * (this->mParent ? this->mParent->getSize().x() : (float)Renderer::getScreenWidth());
	}

	if(properties & FORCE_UPPERCASE && elem->has("forceUppercase"))
		setUppercase(elem->get<bool>("forceUppercase"));

	if(properties & LINE_SPACING)
	{
		if (elem->has("lines"))
		{
			mLineCount = (int)elem->get<float>("lines");
			if (mLineCount > 0)
				setSelectorHeight(mSize.y() / mLineCount);				
		}
		else
			mLineCount = -1;

		if(elem->has("lineSpacing"))
			setLineSpacing(elem->get<float>("lineSpacing"));

		if(elem->has("selectorHeight"))
			setSelectorHeight(elem->get<float>("selectorHeight") * Renderer::getScreenHeight());

		if(elem->has("selectorOffsetY"))
		{
			float scale = this->mParent ? this->mParent->getSize().y() : (float)Renderer::getScreenHeight();
			setSelectorOffsetY(elem->get<float>("selectorOffsetY") * scale);
		} 
		else
			setSelectorOffsetY(0.0);		
	}

	if (elem->has("selectorImagePath"))
	{
		std::string path = elem->get<std::string>("selectorImagePath");
		bool tile = elem->has("selectorImageTile") && elem->get<bool>("selectorImageTile");
		mSelectorImage.setImage(path, tile);
		mSelectorImage.setSize(mSize.x(), mSelectorHeight);
		mSelectorImage.setColorShift(mSelectorColor);
		mSelectorImage.setColorShiftEnd(mSelectorColorEnd);
	} else {
		mSelectorImage.setImage("");
	}
}

#endif // ES_APP_COMPONENTS_TEXT_LIST_COMPONENT_H
