/*
 * mpUtils
 * Handle.h
 *
 * Contains a Handle class to manage handles to openGL objects.
 *
 * @author: Hendrik Schwanekamp
 * @mail:   hendrik.schwanekamp@gmx.net
 *
 * Copyright (c) 2017 Hendrik Schwanekamp
 *
 * This file was originally written and generously provided for this framework from Johannes Braun.
 *
 */
#pragma once

#include <memory>
#include <tuple>
#include <cassert>
#include <GL/glew.h>

namespace mpu {
namespace gph {

	/// An object that wraps a shared_ptr of a handle.
	/// This is used for all OpenGL objects for lifetime control. As in OpenGL, objects are only accessible as references by their ID,
	/// Copying a handle should yield in just a copy of the reference.
	/// To be able to copy the object itself, it usually needs a complete rebuild of the object.<br/>
	///	<br/>
	/// You can fetch the following types from any object deriving from Handle:<br/>
	/// <b>Handle::type</b> The full type of the base handle itself.<br/>
	/// <b>Handle::handle_type</b> The handle type that is wrapped inside. Usually a uint32_t, but can also differ.<br/>
	/// <b>Handle::creator_type</b> The type of the OpenGL object creator function used for this handle type.<br/>
	/// <b>Handle::destroyer_type</b> The type of the OpenGL object destructor function used for this handle type.<br/>
	template<typename THandle, typename TCreate, TCreate Creator, typename TDestroy, TDestroy Destroyer, typename... TArgs>
	class Handle
	{
	public:
		using type = Handle;
		using handle_type = THandle;
		using creator_type = decltype(Creator);
		using destroyer_type = decltype(Destroyer);

		/// Constructor used when having at least one argument.
		/// Will fail if trying to call without stating additional argument types as template parameters.
		template<typename TArg, typename... Args>
		explicit Handle(TArg arg, Args ...args);

		/// Constructs the handle to nullptr.
		/// Useful for delayed assignment.
		explicit Handle(nullptr_t);

		/// If TArgs is empty, this will construct the object referring to the generated handle.
		/// Otherwise, it leaves the handle uninitialized for later assignment or recreation.
		Handle();

		/// Rebuild the object with the given parameters.
		void recreate(TArgs ...args);

		/// Invalidate the handle object to nullptr.
		void recreate(nullptr_t);

		/// Same as recreate(nullptr). Destroys the object and sets the handle to nullptr.
		void destroy();

		// Just for good measure, let's show that this object is copyable and movable as it only
		// represents a handle to an OpenGL object instead of the object itself.
		Handle(const Handle& other) = default;
		Handle(Handle&& other) noexcept = default;
		Handle& operator=(const Handle& other) = default;
		Handle& operator=(Handle&& other) noexcept = default;

		// Conversion from and to uint32_t... and bool for validity checks.
		operator handle_type() const;
		operator bool() const;

	private:
		std::shared_ptr<handle_type> m_handle{ nullptr };
	};

	template<typename THandle, typename TCreate, TCreate Creator, typename TDestroy, TDestroy Destroyer, typename... TArgs>
	template<typename TArg, typename... Args>
	Handle<THandle, TCreate, Creator, TDestroy, Destroyer, TArgs...>::Handle(TArg arg, Args... args)
	{
		// This constructor may only be used if the variadic template argument list contains at least one type.
		// Also, the types passed to this constructor should be equal to the TArgs parameters.
		static_assert(sizeof...(TArgs) != 0 && std::is_convertible_v<std::tuple<TArg, Args...>, std::tuple<TArgs...>>
			, "Invalid argument count or types.");
		recreate(arg, std::forward<Args>(args)...);
	}

	template <typename THandle, typename TCreate, TCreate Creator, typename TDestroy, TDestroy Destroyer, typename ... TArgs>
	Handle<THandle, TCreate, Creator, TDestroy, Destroyer, TArgs...>::Handle(nullptr_t)
	{
		// Keep the handle to nullptr
	}

	template <typename THandle, typename TCreate, TCreate Creator, typename TDestroy, TDestroy Destroyer, typename ... TArgs>
	Handle<THandle, TCreate, Creator, TDestroy, Destroyer, TArgs...>::Handle()
	{
		if constexpr(sizeof...(TArgs) == 0)
		{
			recreate();
		}
		// Else keep the handle as nullptr.
	}

	template <typename THandle, typename TCreate, TCreate Creator, typename TDestroy, TDestroy Destroyer, typename ... TArgs>
	void Handle<THandle, TCreate, Creator, TDestroy, Destroyer, TArgs...>::destroy()
	{
		m_handle.reset();
	}

	template <typename THandle, typename TCreate, TCreate Creator, typename TDestroy, TDestroy Destroyer, typename ... TArgs>
	void Handle<THandle, TCreate, Creator, TDestroy, Destroyer, TArgs...>::recreate(TArgs... args)
	{
		if constexpr(std::is_same_v<decltype(Creator), handle_type(**)(TArgs...)>)
		{
			// First format:
			// creator: auto id = glCreateXYZ(args...);
			// deleter: glDestroyXYZ(id);
			m_handle.reset(new handle_type{ (*Creator)(std::forward<TArgs>(args)...) }, [](handle_type* t) { (*Destroyer)(*t); delete t; });
		}
		else
		{
			// Second format:
			// creator: auto id; glCreateXYZ(1, &id);
			// deleter: glDestroyXYZ(1, &id);
			m_handle.reset(new handle_type{ 0 }, [](handle_type* t) { (*Destroyer)(1, t); delete t; });
			(*Creator)(std::forward<TArgs>(args)..., 1, m_handle.get());
		}
	}

	template <typename THandle, typename TCreate, TCreate Creator, typename TDestroy, TDestroy Destroyer, typename ... TArgs>
	void Handle<THandle, TCreate, Creator, TDestroy, Destroyer, TArgs...>::recreate(nullptr_t)
	{
		destroy();
	}

	template <typename THandle, typename TCreate, TCreate Creator, typename TDestroy, TDestroy Destroyer, typename ... TArgs>
	Handle<THandle, TCreate, Creator, TDestroy, Destroyer, TArgs...>::operator handle_type() const
	{
		assert(m_handle); // do not create a nullpointer handle
		return *m_handle;
	}

	template <typename THandle, typename TCreate, TCreate Creator, typename TDestroy, TDestroy Destroyer, typename ... TArgs>
	Handle<THandle, TCreate, Creator, TDestroy, Destroyer, TArgs...>::operator bool() const
	{
		return static_cast<bool>(m_handle) && *m_handle != 0;
	}
}}