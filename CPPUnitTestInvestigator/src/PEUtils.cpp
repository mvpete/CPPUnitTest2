#include "PEUtils.h"

using namespace PeUtils;

struct CloseHandle2
{
	void operator()(HANDLE h)
	{
		::CloseHandle(h);
	}
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Section
Section::Section(PIMAGE_SECTION_HEADER hdr, LPVOID base, DWORD imageBase)
	:section_(hdr),base_(base),imageBase_(imageBase)
{
}

Section::Section()
	: base_(nullptr), section_(nullptr), imageBase_(0)
{
}

void Section::AssertValid()
{
	if (!IsValid())
		throw std::logic_error("bad section");
}

bool Section::IsValid()
{
	return base_ != nullptr && section_ != nullptr;
}

DWORD Section::SizeOfRawData() const
{
	return section_->SizeOfRawData;
}

DWORD Section::PointerToRawData() const
{
	return reinterpret_cast<DWORD>(base_) + section_->PointerToRawData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PeExplorer
PeExplorer::PeExplorer(const std::string &path)
{
	file_ = CreateExecutioner([&path]()
	{
		HANDLE f = ::CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (f == INVALID_HANDLE_VALUE)
			throw std::runtime_error("unable to open file");
		return f;
	}, CloseHandle2());

	mapping_ = CreateExecutioner([&]()
	{
		HANDLE m = ::CreateFileMapping(file_, NULL, PAGE_READONLY, 0, 0, NULL);
		if (m == 0)
			throw std::runtime_error("failed to create file mapping");
		return m;
	}, CloseHandle2());

	view_ = CreateExecutioner([&]()
	{
		auto pv = ::MapViewOfFile(mapping_, FILE_MAP_READ, 0, 0, 0);
		if (pv == 0)
			throw std::runtime_error("failed to map file view");
		return pv;
	}, [](LPVOID pv)
	{
		::UnmapViewOfFile(pv);
	});

	PIMAGE_DOS_HEADER dos = reinterpret_cast<PIMAGE_DOS_HEADER>(view_.Get());
	PIMAGE_FILE_HEADER img = reinterpret_cast<PIMAGE_FILE_HEADER>(view_.Get());

	if (dos->e_magic == IMAGE_DOS_SIGNATURE)
	{
		type_ = Type::Exe;

		dosh_ = reinterpret_cast<PIMAGE_DOS_HEADER>(view_.Get());
		nth_ = MakePtr<PIMAGE_NT_HEADERS>(view_.Get(), dosh_->e_lfanew);
	}
	else if (img->Machine == IMAGE_FILE_MACHINE_I386 && img->SizeOfOptionalHeader == 0)
	{
		type_ = Type::Obj;
	}

	
}

PeExplorer::Type PeExplorer::GetType() const
{
	return type_;
}

LPCVOID PeExplorer::GetBase() const
{
	return view_.Get();
}

const PIMAGE_FILE_HEADER PeExplorer::GetImageFileHeader() const
{
	switch (GetType())
	{
	case Type::Obj:
		return reinterpret_cast<PIMAGE_FILE_HEADER>(view_.Get());
	case Type::Exe:
	{
		PIMAGE_DOS_HEADER dosh = reinterpret_cast<PIMAGE_DOS_HEADER>(view_.Get());
		PIMAGE_NT_HEADERS nth = MakePtr<PIMAGE_NT_HEADERS>(view_.Get(), dosh->e_lfanew);
		return &nth_->FileHeader;
	}
	default:
		throw std::runtime_error("NOOOOO");
	}
}

WORD PeExplorer::GetSectionCount() const
{
	return GetImageFileHeader()->NumberOfSections;
}

Section PeExplorer::GetSection(const std::string &name) const
{
	

	auto i = IMAGE_FIRST_SECTION(nth_);
	auto i_end = i + GetImageFileHeader()->NumberOfSections;

	auto f = std::find_if(i, i_end, [&name](const IMAGE_SECTION_HEADER &sec)
	{
		if (strnlen((const char*)sec.Name, 8) != name.length())
			return false;
		return strncmp((const char*)sec.Name, name.c_str(), min(name.length(), 8))==0;
	});

	if (f == i_end)
		return Section();

	return Section(f, view_.Get(), nth_->OptionalHeader.ImageBase);
}

Section PeExplorer::GetSection(DWORD ptr) const
{
	DWORD dwPtr = ptr;
	DWORD ibPtr = dwPtr - nth_->OptionalHeader.ImageBase;

	auto i = IMAGE_FIRST_SECTION(nth_);
	auto i_end = i + GetImageFileHeader()->NumberOfSections;

	auto f = std::find_if(i, i_end, [&ibPtr](const IMAGE_SECTION_HEADER &sec)
	{
		return sec.VirtualAddress <= ibPtr && ibPtr <= (sec.VirtualAddress + sec.SizeOfRawData);
	});
	if (f == i_end)
		return Section();
	return Section(f, view_.Get(), nth_->OptionalHeader.ImageBase);
	
}