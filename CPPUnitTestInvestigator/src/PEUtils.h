#ifndef __PE_UTILS_H__
#define __PE_UTILS_H__

#include <Windows.h>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>


namespace PeUtils
{

	template <typename T, typename B>
	constexpr T MakePtr(B base, DWORD offset)
	{
		return reinterpret_cast<T>(reinterpret_cast<DWORD>(base) + offset);
	}

	template <typename T>
	constexpr T MakePtr(DWORD base, DWORD offset)
	{
		return reinterpret_cast<T>(base + offset);
	}

	template <typename T>
	class Executioner
	{
		bool owned_;
		T handle_;
		std::function<void(T)> release_;
	public:
		Executioner()
			:owned_(false)
		{
		}

		Executioner& operator=(Executioner &&rhs)
		{
			std::swap(handle_, rhs.handle_);
			std::swap(release_, rhs.release_);
			std::swap(owned_, rhs.owned_);
			return *this;
		}

		template<typename CreateFn, typename RelFn>
		Executioner(CreateFn &&gen, RelFn &&rel)
			:handle_(gen()),release_(std::move(rel)),owned_(true)
		{
		}

		Executioner(Executioner &&rhs)
			:handle_(rhs.handle_), release_(std::move(rhs.release_)),owned_(true)
		{
			rhs.owned_ = false;
		}
		~Executioner()
		{
			if(owned_)
				release_(handle_);
		}
		operator T()
		{
			return handle_;
		}

		T Get()
		{
			return handle_;
		}
		
		const T Get() const
		{
			return handle_;
		}

	};


	template<typename CreateFn,typename RelFn>
	Executioner<typename std::result_of<CreateFn()>::type> CreateExecutioner(CreateFn &&crt, RelFn &&rels)
	{
		return Executioner<typename std::result_of<CreateFn()>::type>(std::forward<CreateFn>(crt), std::forward<RelFn>(rels));
	}

	class PeExplorer;

	class Section
	{
		PIMAGE_SECTION_HEADER section_;
		LPVOID base_;
		DWORD  imageBase_;
		void AssertValid();

	public:
		Section();
		Section(PIMAGE_SECTION_HEADER sect, LPVOID base, DWORD imageBase);

		template<typename T>
		T PointerToRawData()
		{
			AssertValid();
			return MakePtr<T>(section_->PointerToRawData,reinterpret_cast<DWORD>(base_));
		}

		template <typename T>
		T* AdjustDword(DWORD ptr)
		{
			if (ptr == 0)
				return nullptr;

			AssertValid();
			T *retptr = reinterpret_cast<T*>(ptr - imageBase_ - section_->VirtualAddress + (DWORD)base_ + section_->PointerToRawData);
			return retptr;

		}

		template <typename T>
		T* AdjustPointer(T *ptr) 
		{		
			return AdjustDword<T>(reinterpret_cast<DWORD>(ptr));
		}

		bool IsValid();

		DWORD SizeOfRawData() const;
		DWORD PointerToRawData() const;
	};

	class PeExplorer
	{
	public:
		enum Type { Obj, Exe, Unknown };
	private:
		Type type_;

		Executioner<HANDLE> file_;
		Executioner<HANDLE> mapping_;
		Executioner<LPVOID> view_;
		
		PIMAGE_DOS_HEADER dosh_;
		PIMAGE_NT_HEADERS nth_;

	public:
		
		PeExplorer(const std::string &path);

		Type GetType() const;

		LPCVOID GetBase() const;

		const PIMAGE_FILE_HEADER GetImageFileHeader() const;

		WORD GetSectionCount() const;
		Section GetSection(const std::string &name) const;
		Section GetSection(DWORD ptr) const;

		std::vector<std::string> GetImports() const;

		template<typename T>
		T* FindDwordPtr(DWORD ptr) const
		{
			Section s = GetSection(ptr);
			if (!s.IsValid())
				return nullptr;
			return s.AdjustDword<T>(ptr);
		}

		template<typename T>
		T* FindDwordPtr(DWORD ptr)
		{
			Section s = GetSection(reinterpret_cast<DWORD>(ptr));
			if (!s.IsValid())
				return nullptr;
			return s.AdjustPointer<T>(ptr);
		}
		
		template<typename T>
		T* FindPtr(T *ptr) const
		{
			return FindDwordPtr<T>(reinterpret_cast<DWORD>(ptr));
		}



	};

	template<typename T>
	void AdvancePointer(T*& ptr, DWORD len)
	{
		ptr = reinterpret_cast<T*>(reinterpret_cast<DWORD>(ptr) + len);
	}
}

#endif // __PE_UTILS_H__