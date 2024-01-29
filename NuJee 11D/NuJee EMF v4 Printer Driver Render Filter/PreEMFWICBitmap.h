#pragma once

namespace NuJee_EMF_v4_Printer_Driver_Render_Filter
{
	class PreEMFWICBitmap
	{
	public:
		static HRESULT  SaveWICBitmapToImageFile(IWICBitmap* pWICBitmap, UINT sc_bitmapWidth, UINT sc_bitmapHeight);
		void hello();
	};
}

