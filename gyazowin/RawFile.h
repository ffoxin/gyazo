#ifndef RAWFILE_H
#define RAWFILE_H

#include "types.h"

namespace Gyazo
{
	
	class RawFile
	{
	public:
		RawFile();
		RawFile(const String& file);
		virtual ~RawFile();

		bool Open(const String& file);
		bool Close();

		void Read(uint8_t* buffer, uint32_t size);
		void Write(uint8_t* buffer, uint32_t size);

		int32_t GetPos() const;
		void SetPos(uint32_t offset) const;

	private:
		RawFile(const RawFile& ); // = delete
		RawFile& operator=(const RawFile& ); // = delete

		void OpenFile(const String& file);

	private:
		std::fstream m_file;
	};

}

#endif // RAWFILE_H
