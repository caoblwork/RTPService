#pragma once
#include <string>
using namespace std;

extern "C"
{
#include <libavutil/imgutils.h>
//#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}


#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"swscale.lib")
class PacketReader
{
private:
	AVFormatContext * _fmt_ctx;
	
	AVCodecContext  * _dec_ctx_video ;
	AVCodec *_dec_video ;
	int _stream_idx_video ;
	AVStream *_stream_video ;

	AVCodecContext * _dec_ctx_audio ;
	AVCodec *_dec_audio;
	int _stream_idx_audio;
	AVStream *_stream_audio;
	
	AVFrame * _frame ;
	AVPacket _pkt;
	int frame_count;
	string _src_filename;


public:
	int _width; //得到的视频高度
	int _height;//得到的视频宽度
public:
	PacketReader(void);
	~PacketReader(void);

	bool InitReader(const char * filename);
	void DeInitReader();

	AVCodecContext *getAVCodecContext_Video(){return _dec_ctx_video;}
	AVCodecContext *getAVCodecContext_Audio(){return _dec_ctx_audio;}
	
	int Get_gop_size(){return _dec_ctx_video->gop_size;}
	AVRational* GetVideoFrameRate();
	AVStream * GetDecode_StreamVideo(){return _stream_video;}
	AVStream * GetDecode_StreamAudio(){return _stream_audio;}
	AVPacket * GetPacket(int &audio_video);

	int DecodePacket(const AVPacket * packet);


	AVPacket * GetFirstVideoPacket();
	AVPacket * GetNextVideoPacket();
	void FreePacket(AVPacket * pkt);
	int64_t _first_pts;
};

