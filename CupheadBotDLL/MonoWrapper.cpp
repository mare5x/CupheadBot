#include "MonoWrapper.h"
#include "mono_metadata.h"

MonoWrapper::MonoWrapper()
{
	mono_dll_handle = GetModuleHandle(L"mono.dll");
	
	mono_thread_attach = (MONO_THREAD_ATTACH)GetProcAddress(mono_dll_handle, "mono_thread_attach");
	mono_thread_detach = (MONO_THREAD_DETACH)GetProcAddress(mono_dll_handle, "mono_thread_detach");

	mono_domain_foreach = (MONO_DOMAIN_FOREACH)GetProcAddress(mono_dll_handle, "mono_domain_foreach");
	mono_get_root_domain = (MONO_GET_ROOT_DOMAIN)GetProcAddress(mono_dll_handle, "mono_get_root_domain");

	mono_assembly_foreach = (MONO_ASSEMBLY_FOREACH)GetProcAddress(mono_dll_handle, "mono_assembly_foreach");
	mono_assembly_get_image = (MONO_ASSEMBLY_GET_IMAGE)GetProcAddress(mono_dll_handle, "mono_assembly_get_image");

	mono_image_get_name = (MONO_IMAGE_GET_NAME)GetProcAddress(mono_dll_handle, "mono_image_get_name");
	mono_image_get_table_rows = (MONO_IMAGE_GET_TABLE_ROWS)GetProcAddress(mono_dll_handle, "mono_image_get_table_rows");

	mono_class_from_name = (MONO_CLASS_FROM_NAME)GetProcAddress(mono_dll_handle, "mono_class_from_name");
	mono_class_get_methods = (MONO_CLASS_GET_METHODS)GetProcAddress(mono_dll_handle, "mono_class_get_methods");
	mono_class_get_method_from_name = (MONO_CLASS_GET_METHOD_FROM_NAME)GetProcAddress(mono_dll_handle, "mono_class_get_method_from_name");
	mono_class_get_namespace = (MONO_CLASS_GET_NAMESPACE)GetProcAddress(mono_dll_handle, "mono_class_get_namespace");
	mono_class_get_name = (MONO_CLASS_GET_NAME)GetProcAddress(mono_dll_handle, "mono_class_get_name");
	mono_class_get_parent = (MONO_CLASS_GET_PARENT)GetProcAddress(mono_dll_handle, "mono_class_get_parent");
	mono_class_get = (MONO_CLASS_GET)GetProcAddress(mono_dll_handle, "mono_class_get");

	mono_method_get_name = (MONO_METHOD_GET_NAME)GetProcAddress(mono_dll_handle, "mono_method_get_name");
	mono_method_desc_free = (MONO_METHOD_DESC_FREE)GetProcAddress(mono_dll_handle, "mono_method_desc_free");
	mono_method_desc_new = (MONO_METHOD_DESC_NEW)GetProcAddress(mono_dll_handle, "mono_method_desc_new");
	mono_method_desc_search_in_class = (MONO_METHOD_DESC_SEARCH_IN_CLASS)GetProcAddress(mono_dll_handle, "mono_method_desc_search_in_class");
	mono_method_desc_search_in_image = (MONO_METHOD_DESC_SEARCH_IN_IMAGE)GetProcAddress(mono_dll_handle, "mono_method_desc_search_in_image");
	
	mono_runtime_invoke = (MONO_RUNTIME_INVOKE)GetProcAddress(mono_dll_handle, "mono_runtime_invoke");
	mono_object_unbox = (MONO_OBJECT_UNBOX)GetProcAddress(mono_dll_handle, "mono_object_unbox");

	mono_compile_method = (MONO_COMPILE_METHOD)GetProcAddress(mono_dll_handle, "mono_compile_method");
	mono_jit_info_table_find = (MONO_JIT_INFO_TABLE_FIND)GetProcAddress(mono_dll_handle, "mono_jit_info_table_find");
	mono_jit_info_get_code_start = (MONO_JIT_INFO_GET_CODE_START)GetProcAddress(mono_dll_handle, "mono_jit_info_get_code_start");
}

MonoWrapper::~MonoWrapper()
{
	detach();
}

void MonoWrapper::attach()
{
	mono_thread = mono_thread_attach(mono_get_root_domain());
}

void MonoWrapper::detach()
{
	if (mono_thread) {
		mono_thread_detach(mono_thread);
		mono_thread = nullptr;
	}
}

void __cdecl domain_enumerator(void* domain, std::vector<void*> *user_data)
{
	user_data->push_back(domain);
}

std::vector<void*> MonoWrapper::get_domains()
{
	std::vector<void*> domains;
	mono_domain_foreach((MonoWrapper::MonoDomainFunc)domain_enumerator, &domains);
	return domains;
}

void __cdecl assembly_enumerator(void *domain, std::vector<UINT64> *v)
{
	v->push_back((UINT_PTR)domain);
}

std::vector<UINT64> MonoWrapper::get_assemblies()
{
	std::vector<UINT64> v;
	mono_assembly_foreach((MonoWrapper::GFunc)assembly_enumerator, &v);
	return v;
}

std::vector<void*> MonoWrapper::get_images()
{
	std::vector<void*> v;
	auto assemblies = get_assemblies();
	for (auto assembly : assemblies)
		v.push_back(mono_assembly_get_image((void*)assembly));
	return v;
}

std::vector<void*> MonoWrapper::get_classes(void* image)
{
	// Read the docs for mono metadata access

	std::vector<void*> classes;
	UINT32 rows = mono_image_get_table_rows(image, MONO_TABLE_TYPEDEF);
	for (UINT32 i = 1; i < rows; ++i) {
		void* klass = mono_class_get(image, MONO_TOKEN_TYPE_DEF | (i + 1));
		classes.push_back(klass);
	}
	return classes;
}

void * MonoWrapper::find_image(const char * name)
{
	auto images = get_images();
	for (auto image : images) {
		if (strcmp(name, get_image_name(image)) == 0)
			return image;
	}
	return nullptr;
}

const char * MonoWrapper::get_image_name(void * image)
{
	return mono_image_get_name(image);
}

void * MonoWrapper::get_class(void * image, const char * name, const char * name_space)
{
	return mono_class_from_name(image, name_space, name);
}

void * MonoWrapper::get_class_parent(void * klass)
{
	return mono_class_get_parent(klass);
}

const char * MonoWrapper::get_class_namespace(void * klass)
{
	return mono_class_get_namespace(klass);
}

const char * MonoWrapper::get_class_name(void * klass)
{
	return mono_class_get_name(klass);
}

void MonoWrapper::class_name_builder(void * klass, std::ostringstream & os)
{
	void* parent = get_class_parent(klass);
	if (!parent)
		os << get_class_name(klass);
	else {
		class_name_builder(parent, os);
		os << "." << get_class_name(klass);
	}
}

std::string MonoWrapper::get_full_class_name(void * klass)
{
	if (!klass)
		return "";

	std::ostringstream os;
	class_name_builder(klass, os);
	return os.str();
}

std::vector<void*> MonoWrapper::get_methods(void * klass)
{
	std::vector<void*> methods;
	void* gpointer = NULL;
	void* last_method = mono_class_get_methods(klass, &gpointer);
	while (last_method) {
		methods.push_back(last_method);
		last_method = mono_class_get_methods(klass, &gpointer);
	}
	
	return methods;
}

void * MonoWrapper::get_method_class(void * klass, const char * name, int param_count)
{
	return mono_class_get_method_from_name(klass, name, param_count);
}

void * MonoWrapper::get_method_image(const char * name, void * image)
{
	void* desc = mono_method_desc_new(name, false);
	void* method = mono_method_desc_search_in_image(desc, image);
	mono_method_desc_free(desc);
	return method;
}

const char * MonoWrapper::get_method_name(void * method)
{
	return mono_method_get_name(method);
}

void * MonoWrapper::jit_method(void * method)
{
	return mono_compile_method(method);
}

