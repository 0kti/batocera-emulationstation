#pragma once
#ifndef ES_CORE_RENDERER_RENDERER_H
#define ES_CORE_RENDERER_RENDERER_H

#include <vector>
#include "math/Vector2f.h"

class  Transform4x4f;
class  Vector2i;
struct SDL_Window;

namespace Renderer
{
	namespace Blend
	{
		enum Factor
		{
			ZERO                = 0,
			ONE                 = 1,
			SRC_COLOR           = 2,
			ONE_MINUS_SRC_COLOR = 3,
			SRC_ALPHA           = 4,
			ONE_MINUS_SRC_ALPHA = 5,
			DST_COLOR           = 6,
			ONE_MINUS_DST_COLOR = 7,
			DST_ALPHA           = 8,
			ONE_MINUS_DST_ALPHA = 9

		}; // Factor

	} // Blend::

	namespace Texture
	{
		enum Type
		{
			RGBA  = 0,
			ALPHA = 1

		}; // Type

	} // Texture::

	struct Rect
	{
		Rect(const int _x, const int _y, const int _w, const int _h) : x(_x), y(_y), w(_w), h(_h) { }

		int x;
		int y;
		int w;
		int h;

	}; // Rect

	struct Vertex
	{
		Vertex()                                                                                                      { }
		Vertex(const Vector2f& _pos, const Vector2f& _tex, const unsigned int _col) : pos(_pos), tex(_tex), col(_col) { }

		Vector2f     pos;
		Vector2f     tex;
		unsigned int col;

	}; // Vertex

	class IRenderer
	{
	public:
		virtual std::string getDriverName() = 0;

		virtual std::vector<std::pair<std::string, std::string>> getDriverInformation() = 0;

		virtual unsigned int getWindowFlags() = 0;
		virtual void         setupWindow() = 0;

		virtual void         createContext() = 0;
		virtual void         destroyContext() = 0;

		virtual unsigned int createTexture(const Texture::Type _type, const bool _linear, const bool _repeat, const unsigned int _width, const unsigned int _height, void* _data) = 0;
		virtual void         destroyTexture(const unsigned int _texture) = 0;
		virtual void         updateTexture(const unsigned int _texture, const Texture::Type _type, const unsigned int _x, const unsigned _y, const unsigned int _width, const unsigned int _height, void* _data) = 0;
		virtual void         bindTexture(const unsigned int _texture) = 0;

		virtual void         drawLines(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA) = 0;
		virtual void         drawTriangleStrips(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA, bool verticesChanged = true) = 0;
		virtual void		 drawTriangleFan(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA) = 0;

		virtual void         setProjection(const Transform4x4f& _projection) = 0;
		virtual void         setMatrix(const Transform4x4f& _matrix) = 0;
		virtual void         setViewport(const Rect& _viewport) = 0;
		virtual void         setScissor(const Rect& _scissor) = 0;

		virtual void         setStencil(const Vertex* _vertices, const unsigned int _numVertices) = 0;
		virtual void		 disableStencil() = 0;

		virtual void         setSwapInterval() = 0;
		virtual void         swapBuffers() = 0;
	};
	
	std::vector<std::string> getRendererNames();

 	bool        init            ();
 	void        deinit          ();
	void        pushClipRect    (const Vector2i& _pos, const Vector2i& _size);
	void		pushClipRect	(int x, int y, int w, int h);
	void        popClipRect     ();
	void        drawRect        (const float _x, const float _y, const float _w, const float _h, const unsigned int _color, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA);
	void        drawRect        (const float _x, const float _y, const float _w, const float _h, const unsigned int _color, const unsigned int _colorEnd, bool horizontalGradient = false, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA);

	SDL_Window* getSDLWindow    ();
	int         getWindowWidth  ();
	int         getWindowHeight ();
	int         getScreenWidth  ();
	int         getScreenHeight ();
	int         getScreenOffsetX();
	int         getScreenOffsetY();
	int         getScreenRotate ();
	float		getScreenProportion();

	// API specific
	inline static unsigned int convertColor (const unsigned int _color) { return ((_color & 0xFF000000) >> 24) | ((_color & 0x00FF0000) >> 8) | ((_color & 0x0000FF00) << 8) | ((_color & 0x000000FF) << 24); } 
	// convertColor

	unsigned int getWindowFlags    ();
	void         setupWindow       ();
	void         createContext     ();
	void         destroyContext    ();
	unsigned int createTexture     (const Texture::Type _type, const bool _linear, const bool _repeat, const unsigned int _width, const unsigned int _height, void* _data);
	void         destroyTexture    (const unsigned int _texture);
	void         updateTexture     (const unsigned int _texture, const Texture::Type _type, const unsigned int _x, const unsigned _y, const unsigned int _width, const unsigned int _height, void* _data);
	void         bindTexture       (const unsigned int _texture);
	void         drawLines         (const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA);
	void         drawTriangleStrips(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA, bool verticesChanged = true);
	void         setProjection     (const Transform4x4f& _projection);
	void         setMatrix         (const Transform4x4f& _matrix);
	void         setViewport       (const Rect& _viewport);
	void         setScissor        (const Rect& _scissor);
	void         setSwapInterval   ();
	void         swapBuffers       ();

	std::string  getDriverName();
	std::vector<std::pair<std::string, std::string>> getDriverInformation();

	bool         isClippingEnabled  ();
	bool         isVisibleOnScreen  (float x, float y, float w, float h);
	bool         isSmallScreen      ();
	unsigned int mixColors(unsigned int first, unsigned int second, float percent);

	void		drawRoundRect(float x, float y, float w, float h, float radius, unsigned int color, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA);
	void		enableRoundCornerStencil(float x, float y, float size_x, float size_y, float radius);
	
	void		drawTriangleFan(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor = Blend::SRC_ALPHA, const Blend::Factor _dstBlendFactor = Blend::ONE_MINUS_SRC_ALPHA);

	void		setStencil(const Vertex* _vertices, const unsigned int _numVertices);
	void		disableStencil();

	std::vector<Vertex> createRoundRect(float x, float y, float width, float height, float radius, unsigned int color = 0xFFFFFFFF);

	void		activateWindow();

} // Renderer::

#endif // ES_CORE_RENDERER_RENDERER_H
