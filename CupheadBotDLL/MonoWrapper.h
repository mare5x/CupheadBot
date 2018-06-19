// Sources:
// https://www.mono-project.com/docs/advanced/embedding/
// http://docs.go-mono.com/?link=root:/embed
// cheat-engine/Cheat Engine/bin/autorun/monoscript.lua
// cheat-engine/Cheat Engine/MonoDataCollector/MonoDataCollector/PipeServer.cpp


#pragma once
#include <Windows.h>
#include <vector>
#include <sstream>

class MonoWrapper
{
public:
	MonoWrapper();
	~MonoWrapper();

	/* MUST: call attach before doing any operations in the current thread. (Mono checks if attached already.) */
	void attach();
	void detach();

	std::vector<void*> get_domains();
	std::vector<UINT64> get_assemblies();
	std::vector<void*> get_images();
	std::vector<void*> get_classes(void* image);
	std::vector<void*> get_methods(void* klass);

	void* find_image(const char* name);
	const char* get_image_name(void* image);

	void* get_class(void* image, const char* name, const char* name_space = "");
	void* get_class_parent(void* klass);
	const char* get_class_namespace(void* klass);
	const char* get_class_name(void* klass);
	std::string get_full_class_name(void* klass);

	void* get_method_class(void* klass, const char* name, int param_count = -1);
	void* get_method_image(const char* name, void* image);
	const char* get_method_name(void* method);

	/* This JIT-compiles the method, and returns the pointer to the native code produced. */
	void* jit_method(void* method);
	
	template <typename T>
	T runtime_invoke(void* method, void* obj = NULL, void** params = NULL, void** exc = NULL);
private:
	// Internal functions
	void class_name_builder(void* klass, std::ostringstream& os);

	// Mono boilerplate
	typedef void* (__cdecl *MONO_THREAD_ATTACH)(void *domain);
	typedef void (__cdecl *MONO_THREAD_DETACH)(void *monothread);

	typedef void (__cdecl *MonoDomainFunc)(void* domain, void* user_data);
	typedef void (__cdecl *MONO_DOMAIN_FOREACH)(MonoDomainFunc func, void* user_data);
	typedef void* (__cdecl *MONO_GET_ROOT_DOMAIN)(void);

	typedef void (__cdecl *GFunc)(void* data, void* user_data);
	typedef void* (__cdecl *MONO_ASSEMBLY_FOREACH)(GFunc func, void* data);
	typedef void* (__cdecl *MONO_ASSEMBLY_GET_IMAGE)(void* assembly);

	typedef const char* (__cdecl *MONO_IMAGE_GET_NAME)(void* image);
	typedef int (__cdecl *MONO_IMAGE_GET_TABLE_ROWS)(void* image, int table_id);

	typedef void* (__cdecl *MONO_CLASS_FROM_NAME)(void* image, const char* name_space, const char* name);
	typedef void* (__cdecl *MONO_CLASS_GET_METHODS)(void* klass, void* iter);
	typedef void* (__cdecl *MONO_CLASS_GET_METHOD_FROM_NAME)(void* klass, const char* name, int param_count);
	typedef const char* (__cdecl *MONO_CLASS_GET_NAMESPACE)(void* klass);
	typedef const char* (__cdecl *MONO_CLASS_GET_NAME)(void* klass);
	typedef void* (__cdecl *MONO_CLASS_GET_PARENT)(void* klass);
	typedef void* (__cdecl *MONO_CLASS_GET)(void* image, UINT32 type_token);

	typedef const char* (__cdecl *MONO_METHOD_GET_NAME)(void* method);
	typedef void* (__cdecl *MONO_METHOD_DESC_NEW)(const char *name, bool include_namespace);
	typedef void (__cdecl *MONO_METHOD_DESC_FREE)(void* desc);
	typedef void* (__cdecl *MONO_METHOD_DESC_SEARCH_IN_CLASS)(void* desc, void* klass);
	typedef void* (__cdecl *MONO_METHOD_DESC_SEARCH_IN_IMAGE)(void* desc, void* image);

	typedef void* (__cdecl *MONO_RUNTIME_INVOKE)(void* method, void* obj, void** params, void** exc);
	typedef void* (__cdecl *MONO_OBJECT_UNBOX)(void* obj);

	/* This JIT-compiles the method, and returns the pointer to the native code produced. */
	typedef void* (__cdecl *MONO_COMPILE_METHOD)(void* method);
	typedef void* (__cdecl *MONO_JIT_INFO_TABLE_FIND)(void* domain, char* addr);
	typedef void* (__cdecl *MONO_JIT_INFO_GET_CODE_START)(void* ji);

	MONO_THREAD_ATTACH mono_thread_attach;
	MONO_THREAD_DETACH mono_thread_detach;

	MONO_DOMAIN_FOREACH mono_domain_foreach;
	MONO_GET_ROOT_DOMAIN mono_get_root_domain;

	MONO_ASSEMBLY_FOREACH mono_assembly_foreach;
	MONO_ASSEMBLY_GET_IMAGE mono_assembly_get_image;

	MONO_IMAGE_GET_NAME mono_image_get_name;
	MONO_IMAGE_GET_TABLE_ROWS mono_image_get_table_rows;

	MONO_CLASS_FROM_NAME mono_class_from_name;
	MONO_CLASS_GET_METHODS mono_class_get_methods;
	MONO_CLASS_GET_METHOD_FROM_NAME mono_class_get_method_from_name;
	MONO_CLASS_GET_NAMESPACE mono_class_get_namespace;
	MONO_CLASS_GET_NAME mono_class_get_name;
	MONO_CLASS_GET_PARENT mono_class_get_parent;
	MONO_CLASS_GET mono_class_get;
	
	MONO_METHOD_GET_NAME mono_method_get_name;
	MONO_METHOD_DESC_NEW mono_method_desc_new;  /* [namespace.]classname:methodname[(args...)] */
	MONO_METHOD_DESC_FREE mono_method_desc_free;
	MONO_METHOD_DESC_SEARCH_IN_CLASS mono_method_desc_search_in_class;
	MONO_METHOD_DESC_SEARCH_IN_IMAGE mono_method_desc_search_in_image;
	
	MONO_RUNTIME_INVOKE mono_runtime_invoke;
	MONO_OBJECT_UNBOX mono_object_unbox;

	MONO_COMPILE_METHOD mono_compile_method;
	MONO_JIT_INFO_TABLE_FIND mono_jit_info_table_find;
	MONO_JIT_INFO_GET_CODE_START mono_jit_info_get_code_start;

	// Data
	HMODULE mono_dll_handle;
	void* mono_thread;
};

template<typename T>
inline T MonoWrapper::runtime_invoke(void * method, void * obj, void ** params, void ** exc)
{
	void* result = NULL;
	try {
		result = mono_runtime_invoke(method, obj, params, exc);
	}
	catch (...) {
		result = NULL;
	}
	if (result)
		return *(T*)mono_object_unbox(result);
	return result;
}
