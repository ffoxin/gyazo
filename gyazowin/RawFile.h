#ifndef RAWFILE_H
#define RAWFILE_H

#include "types.h"

namespace Gyazo
{
	
	class RawFile
	{
	public:
		enum Mode_t
		{
			READ, 
			WRITE, 
			APPEND
		};

		RawFile();
		RawFile(const byte_string& file, Mode_t mode);
		RawFile(const RawFile& file);
		virtual ~RawFile();

		RawFile& operator=(const RawFile& file);

		bool Open(const byte_string& file, Mode_t mode);
		bool Close();

		void Read(uint8_t* buffer, uint32_t size);
		void Write(uint8_t* buffer, uint32_t size);

		int32_t GetPos() const;
		void SetPos(uint32_t offset) const;

	private:
		void Swap(const RawFile& file);

	private:
		static const char* Modes[] = {
			"rb", 
			"wb", 
			"ab"
		};

		FILE* m_file;
	};

}

#endif // RAWFILE_H
