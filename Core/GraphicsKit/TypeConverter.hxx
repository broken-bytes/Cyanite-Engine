#pragma once
#include "pch.hxx"
#include <type_traits>

namespace Cyanite::GraphicsKit::Tools {

	/// <summary>
	/// Casts Factory from one type to another
	/// </summary>
	/// <typeparam name="T">The type to use</typeparam>
	/// <param name="factory">The object to cast</param>
	/// <returns>A new com_ptr with the correct type</returns>
	template<
		typename T,
		typename = std::enable_if<std::is_class_v<IDXGIFactory>>
	>
		auto UseAs(winrt::com_ptr<IDXGIFactory> factory) -> T {
		winrt::com_ptr<T> fac;
		fac.attach(reinterpret_cast<T>(factory.detach()));
		factory = nullptr;
		return fac;
	}


	template<
		typename T,
		typename F,
		typename
		std::enable_if<std::is_base_of<IDXGISwapChain, T>::value>::type* = nullptr,
		typename
		std::enable_if<std::is_base_of<IDXGISwapChain, F>::value>::type* = nullptr
	>
		auto UseAs(winrt::com_ptr<F> factory) -> winrt::com_ptr<T> {
		winrt::com_ptr<T> fac;
		fac.attach(reinterpret_cast<T*>(factory.detach()));
		factory = nullptr;
		return fac;
	}

	/// <summary>
/// Casts Factory from one type to another
/// </summary>
/// <typeparam name="T">The type to use</typeparam>
/// <param name="device">The object to cast</param>
/// <returns>A new com_ptr with the correct type</returns>
	template<
		typename T,
		typename = std::enable_if<std::is_class_v<ID3D12Device>>
	>
		auto UseAs(winrt::com_ptr<ID3D12Device> device) -> winrt::com_ptr<T> {
		winrt::com_ptr<T> dev;
		dev.attach(reinterpret_cast<T>(device.detach()));
		device = nullptr;
		return dev;
	}

	/// <summary>
/// Casts CommandList from one type to another
/// </summary>
/// <typeparam name="T">The type to use</typeparam>
/// <param name="list">The object to cast</param>
/// <returns>A new com_ptr with the correct type</returns>
	template<
		typename T,
		typename = std::enable_if<std::is_class_v<ID3D12CommandList>>
	>
		auto UseAs(winrt::com_ptr<ID3D12CommandList> list)
			-> winrt::com_ptr<T> {
		winrt::com_ptr<T> dev;
		dev.attach(reinterpret_cast<T>(list.detach()));
		list = nullptr;
		return dev;
	}
	
}