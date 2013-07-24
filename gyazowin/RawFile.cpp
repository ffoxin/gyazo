#include "RawFile.h"

#include <fstream>

namespace Gyazo
{

	RawFile::RawFile()
		: m_file(0)
	{

	}

	RawFile::RawFile(const byte_string& file, Mode_t mode)
		: m_file(0)
	{
		Open(file, mode);
	}

	RawFile::RawFile(const RawFile& file)
	{
		Swap(file);
	}

	RawFile::~RawFile()
	{
		Close();
	}

	RawFile& RawFile::operator=(const RawFile& file)
	{
		Swap(file);
	}

	bool RawFile::Open(const byte_string& file, Mode_t mode)
	{
		if (m_file != 0)
		{
			Close();
		}

		m_file = fopen(file.c_str(), Modes[mode]);

		return m_file != 0;
	}

	void RawFile::Read(uint8_t* buffer, uint32_t size)
	{
		fread(buffer, sizeof(*buffer), size, m_file);
	}

	void RawFile::Write(uint8_t* buffer, uint32_t size)
	{
		fwrite(buffer, sizeof(*buffer), size, m_file);
	}

	int32_t RawFile::GetPos() const
	{
		return ftell(m_file);
	}

	void RawFile::SetPos(uint32_t offset) const
	{
		fseek(m_file, offset, SEEK_SET);
	}

	void RawFile::Swap(const RawFile& file)
	{
		m_file = file.m_file;
		const_cast<FILE *>(file.m_file) = 0;
	}

} // namespace Gyazo

