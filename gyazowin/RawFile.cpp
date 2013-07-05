#include "RawFile.h"

#include <fstream>

namespace Gyazo
{

	RawFile::RawFile()
	{

	}

	RawFile::RawFile(const String& file)
	{
		OpenFile(file);
	}

	RawFile::~RawFile()
	{
		// "any open file is automatically closed when the fstream object is destroyed"
		// Ref: http://www.cplusplus.com/reference/fstream/fstream/close/
	}

	bool RawFile::Open(const String& file)
	{
		bool result = false;
		if (!m_file.is_open())
		{
			OpenFile(file);
			result = m_file.is_open();
		}

		return result;
	}

	void RawFile::Read(uint8_t* buffer, uint32_t size)
	{
		m_file.read(reinterpret_cast<char *>(buffer), size);
	}

	void RawFile::Write(uint8_t* buffer, uint32_t size)
	{
		m_file.write(reinterpret_cast<char *>(buffer), size);
	}

	int32_t RawFile::GetPos() const
	{

	}

	void RawFile::SetPos(uint32_t offset) const
	{

	}

	RawFile& RawFile::operator=(const RawFile&)
	{

	}

	void RawFile::OpenFile(const String& file)
	{
		m_file.open(file, 
			std::ios_base::in | 
			std::ios_base::out | 
			std::ios_base::binary);
	}

} // namespace Gyazo

