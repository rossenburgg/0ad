#ifndef __ogg_h__
#define __ogg_h__

#include <string>
#include <iostream>
#include <deque>
#include <algorithm>
#include <cassert>

#include <al/al.h>
#include <vorbis/vorbisfile.h>


#include "ogghack.h"

#ifdef _MSC_VER
#pragma comment(lib, "vorbisfile.lib")
#endif


#define BUFFER_SIZE (4096 * 4)




#endif // __ogg_h__


struct Buf
{
	void* p;
	size_t size;
	size_t left;
	size_t pos;
	Buf(void* _p, size_t _size)
	{
		p = _p;
		size = _size;
		left = _size;
		pos = 0;
	}
};

typedef std::deque<Buf> IncomingBufs;




///////////////////////////////////////////////////////////////////////////////
// 
// OGG CALLBACKS
//
///////////////////////////////////////////////////////////////////////////////

size_t read_func(void* ptr, size_t elements, size_t el_size, void* datasource)
{
	IncomingBufs* incoming_bufs = (IncomingBufs*)datasource;

	size_t size = elements*el_size;

	size_t total_read = 0;

	while(!incoming_bufs->empty())
	{
		Buf& b = incoming_bufs->front();
		size_t copy_size = std::min(b.left, size);

		memcpy(ptr, (char*)b.p+b.pos, copy_size);
		total_read += copy_size;
		b.pos += copy_size;
		b.left -= copy_size;
		if(b.left > 0)
			break;

		free(b.p);
		incoming_bufs->pop_front();
	}

	return total_read;
}


int seek_func(void* datasource, ogg_int64_t offset, int whence)
{
	return -1;
}

int close_func(void* datasource)
{
	return 0;
}






struct Ogg
{
	OggVorbis_File  oggStream;
	vorbis_info*    vorbisInfo;
	vorbis_comment* vorbisComment;

	IncomingBufs incoming_bufs;
};


void ogg_give_raw(void* _o, void* p, size_t size)
{
	Ogg* o = (Ogg*)_o;
	IncomingBufs* incoming_bufs = &o->incoming_bufs;

	void* copy = malloc(size);
	memcpy(copy, p, size);
	incoming_bufs->push_back(Buf(copy, size));
}


void* ogg_create()
{
	return new Ogg;
}


void ogg_open(void* _o, ALenum& fmt, ALsizei& freq)
{
	Ogg* o = (Ogg*)_o;
	ov_callbacks cbs = { read_func, seek_func, close_func, 0 };

	void* datasource = &o->incoming_bufs;
	if(ov_open_callbacks(datasource, &o->oggStream, NULL, 0, cbs) < 0)
	{
		assert(!"ov_open failed");
	}

	o->vorbisInfo = ov_info(&o->oggStream, -1);
	o->vorbisComment = ov_comment(&o->oggStream, -1);

	if(o->vorbisInfo->channels == 1)
		fmt = AL_FORMAT_MONO16;
	else
		fmt = AL_FORMAT_STEREO16;

	freq = o->vorbisInfo->rate;
}


void ogg_release(void* _o)
{
	Ogg* o = (Ogg*)_o;
	ov_clear(&o->oggStream);
	delete o;
}



/*
void ogg_display()
{
	cout
		<< "version         " << vorbisInfo->version         << "\n"
		<< "channels        " << vorbisInfo->channels        << "\n"
		<< "rate (hz)       " << vorbisInfo->rate            << "\n"
		<< "bitrate upper   " << vorbisInfo->bitrate_upper   << "\n"
		<< "bitrate nominal " << vorbisInfo->bitrate_nominal << "\n"
		<< "bitrate lower   " << vorbisInfo->bitrate_lower   << "\n"
		<< "bitrate window  " << vorbisInfo->bitrate_window  << "\n"
		<< "\n"
		<< "vendor " << vorbisComment->vendor << "\n";

	for(int i = 0; i < vorbisComment->comments; i++)
		cout << "   " << vorbisComment->user_comments[i] << "\n";

	cout << endl;
}
*/



size_t ogg_read(void* _o, void* buf, size_t max_size)
{
	Ogg* o = (Ogg*)_o;
	size_t bytes_written = 0;

	while(bytes_written < max_size)
	{
		int section;	// unused
		char* pos = (char*)buf + bytes_written;
		size_t left_in_buf = max_size - bytes_written;
		long result = ov_read(&o->oggStream, pos, (int)left_in_buf , 0,2,1, &section);

		if(result > 0)
			bytes_written += result;
		else if(result < 0)
			assert(!"ogg read error");
		// clean break - end of data
		else
			break;
	}

	return bytes_written;
}

