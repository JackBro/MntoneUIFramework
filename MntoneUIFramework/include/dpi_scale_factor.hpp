#pragma once
#include "os_version.hpp"
#include "module.hpp"

namespace mnfx {

class dpi_scale_factor final
{
public:
	using dpi_unit = float;

	dpi_scale_factor() noexcept
		: enabled_(false)
	{
		if (os_version_ >= os_version::eight_one)
		{
			HRESULT hr = S_OK;
			module_ = ::std::make_shared<module>(L"shcore.dll", hr);
			if (SUCCEEDED(hr))
			{
				get_dpi_for_monitor_ = reinterpret_cast<GetDpiForMonitor*>(GetProcAddress(module_->handle(), "GetDpiForMonitor"));
				if (get_dpi_for_monitor_ != nullptr) enabled_ = true;
			}
		}
	}

	void initialize(HMONITOR hmonitor) noexcept
	{
		if (enabled_)
		{
			UINT y, x;
			get_dpi_for_monitor_(hmonitor, 0, &x, &y);
			ydpi_ = static_cast<dpi_unit>(y) / 96;
			xdpi_ = static_cast<dpi_unit>(x) / 96;
		}
		else
		{
			HDC hdc = GetDC(nullptr);
			ydpi_ = static_cast<dpi_unit>(GetDeviceCaps(hdc, LOGPIXELSY)) / 96;
			xdpi_ = static_cast<dpi_unit>(GetDeviceCaps(hdc, LOGPIXELSX)) / 96;
			ReleaseDC(nullptr, hdc);
		}
	}

	::std::pair<dpi_unit, dpi_unit> set_dpi(uint16_t y, uint16_t x) noexcept
	{
		dpi_unit new_ydpi, new_xdpi;
		new_ydpi = static_cast<dpi_unit>(y) / 96;
		new_xdpi = static_cast<dpi_unit>(x) / 96;

		float scale_factor_y, scale_factor_x;
		scale_factor_y = new_ydpi / ydpi_;
		scale_factor_x = new_xdpi / xdpi_;

		ydpi_ = new_ydpi;
		xdpi_ = new_xdpi;

		return ::std::make_pair(scale_factor_y, scale_factor_x);
	}

	template<typename T>
	T scale_y(T virtual_y) const noexcept
	{
		return static_cast<T>(ydpi_ * static_cast<dpi_unit>(virtual_y));
	}

	template<typename T>
	T scale_x(T virtual_x) const noexcept
	{
		return static_cast<T>(xdpi_ * static_cast<dpi_unit>(virtual_x));
	}

	template<typename T>
	T scale_inverse_y(T physical_y) const noexcept
	{
		return static_cast<T>(static_cast<dpi_unit>(physical_y) / ydpi_);
	}

	template<typename T>
	T scale_inverse_x(T physical_x) const noexcept
	{
		return static_cast<T>(static_cast<dpi_unit>(physical_x) / xdpi_);
	}

private:
	dpi_scale_factor(dpi_scale_factor const&) = delete;

	dpi_scale_factor& operator=(dpi_scale_factor const&) = delete;

private:
	dpi_unit ydpi_, xdpi_;
	os_version os_version_;

	using GetDpiForMonitor = HRESULT __stdcall(HMONITOR hmonitor, int dpiType, UINT* dpiX, UINT* dpiY);
	bool enabled_;
	::std::shared_ptr<module> module_;
	GetDpiForMonitor* get_dpi_for_monitor_;
};

}