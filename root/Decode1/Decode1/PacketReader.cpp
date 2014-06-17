#include "StdAfx.h"
#include "PacketReader.h"


PacketReader::PacketReader(void)
{
	_fmt_ctx = NULL;
	_dec_ctx_video = NULL;
	_dec_ctx_audio = NULL;
	_dec_video     = NULL;
	_dec_audio     = NULL;
	_stream_video  = NULL;
	_stream_audio  = NULL;
	_frame   = NULL;
	_width    = 0;
	_height   = 0;
	_stream_idx_video = -1;
	_stream_idx_audio = -1;

}


PacketReader::~PacketReader(void)
{
	//UnInit();
}

void PacketReader::DeInitReader()
{
	avcodec_close(_dec_ctx_video);
	avcodec_close(_dec_ctx_audio);
	av_close_input_file(_fmt_ctx);
	//avformat_close_input(&_fmt_ctx);
}
bool PacketReader::InitReader(const char * filename)
{
	//const char *src_filename = "rtmp://localhost/live/black live=1"; 
	_src_filename = filename;

	/* register all formats and codecs */
	/* open input file, and allocated format context */
	if (avformat_open_input(&_fmt_ctx, _src_filename.c_str(), NULL, NULL) < 0) {
		fprintf(stderr, "Could not open source file %s\n", _src_filename.c_str());
		return false;
	}

	/* retrieve stream information */
	if (avformat_find_stream_info(_fmt_ctx, NULL) < 0) {
		fprintf(stderr, "Could not find stream information\n");
		exit(1);
	}
	av_dump_format(_fmt_ctx,0,_src_filename.c_str(),0);//列出输入文件的相关流信息
	int ret = av_find_best_stream(_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not find video stream in file\n");
		return false;
	}


	_stream_idx_video = ret;
	_stream_video     = _fmt_ctx->streams[_stream_idx_video];
	_dec_ctx_video    = _stream_video->codec;
	_width            = _dec_ctx_video->width;
	_height           = _dec_ctx_video->height;



	_dec_video=avcodec_find_decoder(_dec_ctx_video->codec_id);
	if(_dec_video==NULL)
	{
		printf("can't find suitable video decoder\n");
		exit(1);
	}//找到合适的视频解码器


	if(avcodec_open2(_dec_ctx_video,_dec_video,NULL)<0)
	{
		printf("can't open the video decoder\n");
		exit(1);
	}

	ret = av_find_best_stream(_fmt_ctx,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);
	if(ret<0)
	{
		fprintf(stderr,"no audio\n");
	}
	else
	{
		_stream_idx_audio = ret;
	}
	_stream_audio = _fmt_ctx->streams[_stream_idx_audio];
	_dec_ctx_audio = _stream_audio->codec;
	
	_dec_audio = avcodec_find_decoder(_dec_ctx_audio->codec_id);
	if(_dec_audio == NULL)
	{
		printf("can't find suitable audio decoder\n");
	}
	if(avcodec_open2(_dec_ctx_audio,_dec_audio,NULL)<0)
	{
		printf("can't open the audio decoder\n");
		exit(1);
	}//打开该视频解码器


	av_init_packet(&_pkt);
	_pkt.size = 0;
	_pkt.data = NULL;



	return true;

}

AVRational* PacketReader::GetVideoFrameRate()
{
	//ic->streams[videoindex]->r_frame_rate

	return &_stream_video->r_frame_rate;
}
//audio = 0 video = 1
AVPacket * PacketReader::GetPacket(int &audio_video)
{
	if(av_read_frame(_fmt_ctx, &_pkt) >= 0)
	{

		if(_pkt.stream_index == _stream_idx_video) //视频
		{
			
			//printf("the video packet's pts is %lld\n",_pkt.pts);
			audio_video = 1;
			return &_pkt;
		}
		else if(_pkt.stream_index  == _stream_idx_audio)//音频
		{
			//printf("the audio packet's pts is %lld\n",_pkt.pts);
			audio_video = 0;
			return &_pkt;
		}
		else
		{
			av_free_packet(&_pkt);
		}
		//return &_pkt;
	}
	else
	{
		audio_video = -1; //文件结束或者出错，退出
	}
	return NULL;
}

//得到第一个视频I帧
AVPacket * PacketReader::GetFirstVideoPacket()
{
	
	while(av_read_frame(_fmt_ctx, &_pkt) >= 0)
	{
		if(_pkt.stream_index == _stream_idx_video) //视频
		{
			if(_pkt.flags & AV_PKT_FLAG_KEY )
			{
				_first_pts = _pkt.pts; //保存得到的第一个视频pts值
				printf("the value is %lld\n",_first_pts );
				return &_pkt;
			}
			else
				av_free_packet(&_pkt);
		}
		else
		{
			av_free_packet(&_pkt);
		}
	}
	return NULL;
}
AVPacket *PacketReader::GetNextVideoPacket()
{
	while(av_read_frame(_fmt_ctx,&_pkt)>=0)
	{
		if(_pkt.stream_index == _stream_idx_video) //视频
		{
			return &_pkt;
		}
		else
		{
			av_free_packet(&_pkt);
		}
	}
	return NULL;
}

void PacketReader::FreePacket(AVPacket * pkt)
{
	av_free_packet(pkt);
}



int PacketReader::DecodePacket(const AVPacket * packet)
{
	AVFrame oVFrame;
	int len;
	int frameFinished;

		len = avcodec_decode_video2(_dec_ctx_video, &oVFrame, &frameFinished, packet);
			//len=avcodec_decode_video(vCodecCtx,oVFrame,&frameFinished,packet.data,packet.size);//若为视频包，解码该视频包
			if(len<0)
			{
				printf("Error while decoding\n");
				exit(0);
			}

			if(frameFinished)//判断视频祯是否读完
			{
			}
}
