#ifndef TEXT_TOOL_STATE_PLATFORM_DELEGATE_H
#define TEXT_TOOL_STATE_PLATFORM_DELEGATE_H


#include "TextToolState.h"

#include <Shape.h>


class TextToolState::PlatformDelegate {
public:
	PlatformDelegate(TextToolState* state)
		:
		fState(state)
	{
	}

	void DrawControls(PlatformDrawContext& drawContext, const BPoint& origin,
		const BPoint& widthOffset)
	{
		QPainter& painter = drawContext.Painter();
		painter.setRenderHint(QPainter::Antialiasing, true);

		painter.setPen(QPen(QColor(0, 0, 0, 200), 3));
		painter.drawLine(origin, widthOffset);
		painter.setPen(QPen(QColor(255, 255, 255, 200), 1));
		painter.drawLine(origin, widthOffset);

		float size = 4;
		painter.setPen(QColor(0, 0, 0, 200));
		painter.setBrush(QColor(255, 255, 255, 200));
		painter.drawEllipse(origin, size, size);

		size = 2;
		painter.drawRect(QRectF(widthOffset.x - size, widthOffset.y - size,
			size * 2 + 1, size * 2 + 1));
	}


	void DrawInvertedShape(PlatformDrawContext& drawContext, BShape& shape)
	{
		QPainter painter(drawContext.View());
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor(255, 255, 255));
		painter.drawPath(shape.PainterPath());
	}

private:
	TextToolState*	fState;
};


#endif // TEXT_TOOL_STATE_PLATFORM_DELEGATE_H
