//-------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Xbox Series X|S Direct3D 12.XS constants, structures, and interfaces.
//

#define __XBOX_D3D12_VIDEO_X__


#include "d3d12_xs.h"

#ifndef __d3d12_video_x_h__
#define __d3d12_video_x_h__

#pragma once

#pragma warning( push )

// Disable unreferenced parameter, nameless struct/union, zero-sized array, structure padded 
// due to __declspec(align()), conditional expression is constant, unreachable code, 'symbol'
// is not defined as a preprocessor macro, replacing with '0' for 'directives', implicit
// padding

#pragma warning( disable : 4100 4201 4200 4324 4127 4702 4668 4820 )


extern "C"
{

//
// Forward Declarations
//

// 12.0

struct ID3D12VideoDevice;
struct ID3D12VideoDecodeCommandList;
struct ID3D12VideoProcessCommandList;
struct ID3D12VideoDecoder;
struct ID3D12VideoDecoderHeap;
struct ID3D12VideoProcessor;
struct ID3D12VideoProcessStream;
struct D3D12_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS;
struct D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS;

}


extern "C"
{
    typedef 
    enum D3D12_VIDEO_FRAME_FORMAT
        {
            D3D12_VIDEO_FRAME_FORMAT_PROGRESSIVE    = 0,
            D3D12_VIDEO_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST = 1,
            D3D12_VIDEO_FRAME_FORMAT_INTERLACED_BOTTOM_FIELD_FIRST  = 2,
        }   D3D12_VIDEO_FRAME_FORMAT;
    
    typedef 
    enum D3D12_VIDEO_FRAME_STEREO_FORMAT
        {
            D3D12_VIDEO_FRAME_STEREO_FORMAT_NONE    = 0,
            D3D12_VIDEO_FRAME_STEREO_FORMAT_MONO    = 1,
            D3D12_VIDEO_FRAME_STEREO_FORMAT_HORIZONTAL  = 2,
            D3D12_VIDEO_FRAME_STEREO_FORMAT_VERTICAL    = 3,
            D3D12_VIDEO_FRAME_STEREO_FORMAT_SEPARATE    = 4,
        }   D3D12_VIDEO_FRAME_STEREO_FORMAT;
    
    typedef struct D3D12_VIDEO_FORMAT
        {
        DXGI_FORMAT Format;
        DXGI_COLOR_SPACE_TYPE ColorSpace;
        }   D3D12_VIDEO_FORMAT;
    
    typedef struct D3D12_VIDEO_SAMPLE
        {
        UINT Width;
        UINT Height;
        D3D12_VIDEO_FORMAT Format;
        }   D3D12_VIDEO_SAMPLE;
    
    typedef 
    enum D3D12_VIDEO_USAGE
        {
            D3D12_VIDEO_USAGE_NORMAL    = 0,
            D3D12_VIDEO_USAGE_POWER = 1,
            D3D12_VIDEO_USAGE_QUALITY   = 2,
        }   D3D12_VIDEO_USAGE;
    
    typedef 
    enum D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE
        {
            D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE   = 0,
            D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_FIELD_BASED    = 1,
        }   D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE;
    
    typedef 
    enum D3D12_FEATURE_VIDEO
        {
            D3D12_FEATURE_VIDEO_DECODE_SUPPORT = 0,
            D3D12_FEATURE_VIDEO_DECODE_PROFILES = 1,
            D3D12_FEATURE_VIDEO_DECODE_FORMATS = 2,
            D3D12_FEATURE_VIDEO_DECODE_CONVERSION_SUPPORT = 3,
            D3D12_FEATURE_VIDEO_PROCESS_SUPPORT = 5,
            D3D12_FEATURE_VIDEO_PROCESS_MAX_INPUT_STREAMS = 6,
            D3D12_FEATURE_VIDEO_PROCESS_REFERENCE_INFO = 7,
            D3D12_FEATURE_VIDEO_DECODER_HEAP_SIZE = 8,
            D3D12_FEATURE_VIDEO_PROCESSOR_SIZE = 9,
            D3D12_FEATURE_VIDEO_DECODE_PROFILE_COUNT = 10,
            D3D12_FEATURE_VIDEO_DECODE_FORMAT_COUNT = 11,
            D3D12_FEATURE_VIDEO_ARCHITECTURE = 17,
            D3D12_FEATURE_VIDEO_DECODE_HISTOGRAM = 18,
        }   D3D12_FEATURE_VIDEO;
    

        typedef enum D3D12_BITSTREAM_ENCRYPTION_TYPE
        {
            D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE = 0,
        } D3D12_BITSTREAM_ENCRYPTION_TYPE;
        
        typedef struct D3D12_VIDEO_DECODE_CONFIGURATION
        {
        GUID DecodeProfile;
        D3D12_BITSTREAM_ENCRYPTION_TYPE BitstreamEncryption;
        D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE InterlaceType;
        }   D3D12_VIDEO_DECODE_CONFIGURATION;
    
        typedef struct D3D12_VIDEO_DECODER_DESC
        {
            UINT NodeMask;
            D3D12_VIDEO_DECODE_CONFIGURATION Configuration;
        } D3D12_VIDEO_DECODER_DESC;

        typedef struct D3D12_VIDEO_DECODER_HEAP_DESC
        {
            UINT NodeMask;
            D3D12_VIDEO_DECODE_CONFIGURATION Configuration;
            UINT DecodeWidth;
            UINT DecodeHeight;
            DXGI_FORMAT Format;
            DXGI_RATIONAL FrameRate;
            UINT BitRate;
            UINT MaxDecodePictureBufferCount;
        } D3D12_VIDEO_DECODER_HEAP_DESC;


        typedef struct D3D12_VIDEO_SIZE_RANGE
        {
            UINT MaxWidth;
            UINT MaxHeight;
            UINT MinWidth;
            UINT MinHeight;
        } D3D12_VIDEO_SIZE_RANGE;

        typedef struct D3D12_VIDEO_PROCESSOR_DESC
        {
        UINT NodeMask;
        D3D12_VIDEO_USAGE Usage;
        }   D3D12_VIDEO_PROCESSOR_DESC;

        typedef
            enum D3D12_VIDEO_SCALE_SUPPORT_FLAGS
        {
            D3D12_VIDEO_SCALE_SUPPORT_FLAG_NONE = 0,
            D3D12_VIDEO_SCALE_SUPPORT_FLAG_POW2_ONLY = 0x1,
            D3D12_VIDEO_SCALE_SUPPORT_FLAG_EVEN_DIMENSIONS_ONLY = 0x2,
        }   D3D12_VIDEO_SCALE_SUPPORT_FLAGS;

        typedef struct D3D12_VIDEO_SCALE_SUPPORT
        {
            D3D12_VIDEO_SIZE_RANGE OutputSizeRange;
            D3D12_VIDEO_SCALE_SUPPORT_FLAGS Flags;
        } D3D12_VIDEO_SCALE_SUPPORT;

}

extern "C"
{

    //
    // ID3D12VideoDecodeHeap
    //

    struct D3D12VideoDecoderHeap_Function_Table
    {
        D3D12Pageable_Function_Table ObjectFunction;

        D3D12_VIDEO_DECODER_HEAP_DESC(D3DAPI *GetDesc)(ID3D12VideoDecoderHeap * pThis);

    };


#if DECLARE_UMD_EXPORTS_D3D12X

    D3D12_VIDEO_DECODER_HEAP_DESC D3DAPI D3D12VideoDecoderHeap_GetDesc(ID3D12VideoDecoderHeap * pThis);
    ULONG D3DAPI D3d12VideoDecoderHeap_Release(_Inout_ IGraphicsUnknown* pUnknown);

    D3DCONST D3D12VideoDecoderHeap_Function_Table D3D__D3D12VideoDecoderHeap_Function_Table =
    {
        {
            {
                {
                    {
                        D3d12Object_QueryInterface,
                        D3d12DeviceChild_AddRef,
                        D3d12VideoDecoderHeap_Release,
                    },
                    D3d12Object_GetPrivateData,
                    D3d12Object_SetPrivateData,
                    D3d12Object_SetPrivateDataInterface,
                    D3d12Object_SetName,
                },

                D3d12DeviceChild_GetDevice,
            },
        },
        D3D12VideoDecoderHeap_GetDesc,
    };
#endif

    // external c
};


D3DINTERFACE(ID3D12VideoDecoderHeap, 0946b7c9, ebf6, 4047, bb, 73, 86, 83, e2, 7d, bb, 1f) : public ID3D12Pageable
{
public:

    D3DINLINE D3D12VideoDecoderHeap_Function_Table* DecoderHeapFunction()
    {
        return (D3D12VideoDecoderHeap_Function_Table*)m_pFunction;
    }

    D3DINLINE D3D12_VIDEO_DECODER_HEAP_DESC D3DAPI GetDesc()
    {
        return DecoderHeapFunction()->GetDesc(this);
    }
};



extern "C"

{
    typedef
        enum D3D12_VIDEO_FIELD_TYPE
    {
        D3D12_VIDEO_FIELD_TYPE_PROGRESSIVE = 0,
        D3D12_VIDEO_FIELD_TYPE_INTERLACED_TOP_FIELD_FIRST = 1,
        D3D12_VIDEO_FIELD_TYPE_INTERLACED_BOTTOM_FIELD_FIRST = 2,
        D3D12_VIDEO_FIELD_TYPE_MAX_VALID = 3
    }   D3D12_VIDEO_FIELD_TYPE;


    typedef
        enum D3D12_VIDEO_PROCESS_FEATURE_FLAGS
    {
        D3D12_VIDEO_PROCESS_FEATURE_FLAG_NONE = 0,
        D3D12_VIDEO_PROCESS_FEATURE_FLAG_ALPHA_FILL = 0x1,
        D3D12_VIDEO_PROCESS_FEATURE_FLAG_LUMA_KEY = 0x2,
        D3D12_VIDEO_PROCESS_FEATURE_FLAG_STEREO = 0x4,
        D3D12_VIDEO_PROCESS_FEATURE_FLAG_ROTATION = 0x8,
        D3D12_VIDEO_PROCESS_FEATURE_FLAG_FLIP = 0x10,
        D3D12_VIDEO_PROCESS_FEATURE_FLAG_ALPHA_BLENDING = 0x20,
        D3D12_VIDEO_PROCESS_FEATURE_FLAG_PIXEL_ASPECT_RATIO = 0x40,
        D3D12_VIDEO_PROCESS_FEATURE_FLAG_MASK = 0x7f
    }   D3D12_VIDEO_PROCESS_FEATURE_FLAGS;

    DEFINE_ENUM_FLAG_OPERATORS(D3D12_VIDEO_PROCESS_FEATURE_FLAGS);
    typedef
        enum D3D12_VIDEO_PROCESS_FILTER
    {
        D3D12_VIDEO_PROCESS_FILTER_BRIGHTNESS = 0,
        D3D12_VIDEO_PROCESS_FILTER_CONTRAST = 1,
        D3D12_VIDEO_PROCESS_FILTER_HUE = 2,
        D3D12_VIDEO_PROCESS_FILTER_SATURATION = 3,
        D3D12_VIDEO_PROCESS_FILTER_NOISE_REDUCTION = 4,
        D3D12_VIDEO_PROCESS_FILTER_EDGE_ENHANCEMENT = 5,
        D3D12_VIDEO_PROCESS_FILTER_ANAMORPHIC_SCALING = 6,
        D3D12_VIDEO_PROCESS_FILTER_STEREO_ADJUSTMENT = 7
    }   D3D12_VIDEO_PROCESS_FILTER;

    typedef
        enum D3D12_VIDEO_PROCESS_FILTER_FLAGS
    {
        D3D12_VIDEO_PROCESS_FILTER_FLAG_NONE = 0,
        D3D12_VIDEO_PROCESS_FILTER_FLAG_BRIGHTNESS = (1 << D3D12_VIDEO_PROCESS_FILTER_BRIGHTNESS),
        D3D12_VIDEO_PROCESS_FILTER_FLAG_CONTRAST = (1 << D3D12_VIDEO_PROCESS_FILTER_CONTRAST),
        D3D12_VIDEO_PROCESS_FILTER_FLAG_HUE = (1 << D3D12_VIDEO_PROCESS_FILTER_HUE),
        D3D12_VIDEO_PROCESS_FILTER_FLAG_SATURATION = (1 << D3D12_VIDEO_PROCESS_FILTER_SATURATION),
        D3D12_VIDEO_PROCESS_FILTER_FLAG_NOISE_REDUCTION = (1 << D3D12_VIDEO_PROCESS_FILTER_NOISE_REDUCTION),
        D3D12_VIDEO_PROCESS_FILTER_FLAG_EDGE_ENHANCEMENT = (1 << D3D12_VIDEO_PROCESS_FILTER_EDGE_ENHANCEMENT),
        D3D12_VIDEO_PROCESS_FILTER_FLAG_ANAMORPHIC_SCALING = (1 << D3D12_VIDEO_PROCESS_FILTER_ANAMORPHIC_SCALING),
        D3D12_VIDEO_PROCESS_FILTER_FLAG_STEREO_ADJUSTMENT = (1 << D3D12_VIDEO_PROCESS_FILTER_STEREO_ADJUSTMENT),
        D3D12_VIDEO_PROCESS_FILTER_FLAG_MASK = 0xff
    }   D3D12_VIDEO_PROCESS_FILTER_FLAGS;
    DEFINE_ENUM_FLAG_OPERATORS(D3D12_VIDEO_PROCESS_FILTER_FLAGS);

#define	D3D12_VIDEO_PROCESS_MAX_FILTERS	( 32 )

    typedef enum D3D12_VIDEO_PROCESS_DEINTERLACE_FLAGS
    {
        D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_NONE = 0x0,
        D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_BOB = 0x1,
        D3D12_VIDEO_PROCESS_DEINTERLACE_FLAG_CUSTOM = 0x80000000,

    } D3D12_VIDEO_PROCESS_DEINTERLACE_FLAGS;
    DEFINE_ENUM_FLAG_OPERATORS(D3D12_VIDEO_PROCESS_DEINTERLACE_FLAGS);

    typedef struct D3D12_VIDEO_PROCESS_ALPHA_BLENDING
    {
        BOOL Enable;
        FLOAT Alpha;
    } D3D12_VIDEO_PROCESS_ALPHA_BLENDING;

    typedef struct D3D12_VIDEO_PROCESS_LUMA_KEY
    {
        BOOL Enable;
        FLOAT Lower;
        FLOAT Upper;
    } D3D12_VIDEO_PROCESS_LUMA_KEY;

    typedef struct D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC
    {
        DXGI_FORMAT Format;
        DXGI_COLOR_SPACE_TYPE ColorSpace;
        DXGI_RATIONAL SourceAspectRatio;
        DXGI_RATIONAL DestinationAspectRatio;
        DXGI_RATIONAL FrameRate;
        D3D12_VIDEO_SIZE_RANGE SourceSizeRange;
        D3D12_VIDEO_SIZE_RANGE DestinationSizeRange;
        BOOL EnableOrientation;
        D3D12_VIDEO_PROCESS_FILTER_FLAGS FilterFlags;
        D3D12_VIDEO_FRAME_STEREO_FORMAT StereoFormat;
        D3D12_VIDEO_FIELD_TYPE FieldType;
        D3D12_VIDEO_PROCESS_DEINTERLACE_FLAGS DeinterlaceMode;
        BOOL EnableAlphaBlending;
        D3D12_VIDEO_PROCESS_LUMA_KEY LumaKey;
        UINT NumPastFrames;
        UINT NumFutureFrames;
        BOOL EnableAutoProcessing;
    } D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC;

    typedef enum D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE
    {
        D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_OPAQUE = 0,
        D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_BACKGROUND = 1,
        D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_DESTINATION = 2,
        D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE_SOURCE_STREAM = 3,
    } D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE;

    typedef struct D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC
    {
        DXGI_FORMAT Format;
        DXGI_COLOR_SPACE_TYPE ColorSpace;
        D3D12_VIDEO_PROCESS_ALPHA_FILL_MODE AlphaFillMode;
        UINT AlphaFillModeSourceStreamIndex;
        FLOAT BackgroundColor[4];
        DXGI_RATIONAL FrameRate;
        BOOL EnableStereo;
    } D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC;

    typedef
        enum D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAGS
    {
        D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_NONE = 0,
        D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_DENOISE = 0x1,
        D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_DERINGING = 0x2,
        D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_EDGE_ENHANCEMENT = 0x4,
        D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_COLOR_CORRECTION = 0x8,
        D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_FLESH_TONE_MAPPING = 0x10,
        D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_IMAGE_STABILIZATION = 0x20,
        D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_SUPER_RESOLUTION = 0x40,
        D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_ANAMORPHIC_SCALING = 0x80,
        D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_CUSTOM = 0x100,
        D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAG_MASK = 0x1ff
    }   D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAGS;

    DEFINE_ENUM_FLAG_OPERATORS(D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAGS);

    typedef
        enum D3D12_VIDEO_PROCESS_ORIENTATION
    {
        D3D12_VIDEO_PROCESS_ORIENTATION_DEFAULT = 0,
        D3D12_VIDEO_PROCESS_ORIENTATION_FLIP_HORIZONTAL = 1,
        D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_90 = 2,
        D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_90_FLIP_HORIZONTAL = 3,
        D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_180 = 4,
        D3D12_VIDEO_PROCESS_ORIENTATION_FLIP_VERTICAL = 5,
        D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_270 = 6,
        D3D12_VIDEO_PROCESS_ORIENTATION_CLOCKWISE_270_FLIP_HORIZONTAL = 7,
        D3D12_VIDEO_PROCESS_ORIENTATION_MAX_VALID = 8
    }   D3D12_VIDEO_PROCESS_ORIENTATION;


    typedef
        enum D3D12_VIDEO_PROCESS_INPUT_STREAM_FLAGS
    {
        D3D12_VIDEO_PROCESS_INPUT_STREAM_FLAG_NONE = 0,
        D3D12_VIDEO_PROCESS_INPUT_STREAM_FLAG_FRAME_DISCONTINUITY = 0x1,
        D3D12_VIDEO_PROCESS_INPUT_STREAM_FLAG_FRAME_REPEAT = 0x2,
        D3D12_VIDEO_PROCESS_INPUT_STREAM_FLAG_MAX = 0x3
    }   D3D12_VIDEO_PROCESS_INPUT_STREAM_FLAGS;

    DEFINE_ENUM_FLAG_OPERATORS(D3D12_VIDEO_PROCESS_INPUT_STREAM_FLAGS);

    typedef struct D3D12_VIDEO_PROCESS_FILTER_RANGE
    {
        INT Minimum;
        INT Maximum;
        INT Default;
        FLOAT Multiplier;
    }   D3D12_VIDEO_PROCESS_FILTER_RANGE;

    typedef
        enum D3D12_VIDEO_PROCESS_SUPPORT_FLAGS
    {
        D3D12_VIDEO_PROCESS_SUPPORT_FLAG_NONE = 0,
        D3D12_VIDEO_PROCESS_SUPPORT_FLAG_SUPPORTED = 0x1,
        D3D12_VIDEO_PROCESS_SUPPORT_FLAG_MASK = 0x1
    }   D3D12_VIDEO_PROCESS_SUPPORT_FLAGS;

    DEFINE_ENUM_FLAG_OPERATORS(D3D12_VIDEO_PROCESS_SUPPORT_FLAGS);

    typedef struct D3D12_FEATURE_DATA_VIDEO_PROCESS_SUPPORT
    {
        UINT NodeIndex;                                                                         // input
        D3D12_VIDEO_SAMPLE InputSample;                                                         // input
        D3D12_VIDEO_FIELD_TYPE InputFieldType;                                                  // input
        D3D12_VIDEO_FRAME_STEREO_FORMAT InputStereoFormat;                                      // input
        DXGI_RATIONAL InputFrameRate;                                                           // input
        D3D12_VIDEO_FORMAT OutputFormat;                                                        // input
        D3D12_VIDEO_FRAME_STEREO_FORMAT OutputStereoFormat;                                     // input
        DXGI_RATIONAL OutputFrameRate;                                                          // input
        D3D12_VIDEO_PROCESS_SUPPORT_FLAGS SupportFlags;                                         // output
        D3D12_VIDEO_SCALE_SUPPORT ScaleSupport;                                                 // output
        D3D12_VIDEO_PROCESS_FEATURE_FLAGS FeatureSupport;                                       // output
        D3D12_VIDEO_PROCESS_DEINTERLACE_FLAGS DeinterlaceSupport;                               // output
        D3D12_VIDEO_PROCESS_AUTO_PROCESSING_FLAGS AutoProcessingSupport;                        // output
        D3D12_VIDEO_PROCESS_FILTER_FLAGS FilterSupport;                                         // output
        D3D12_VIDEO_PROCESS_FILTER_RANGE FilterRangeSupport[D3D12_VIDEO_PROCESS_MAX_FILTERS];   // output
    } D3D12_FEATURE_DATA_VIDEO_PROCESS_SUPPORT;

    typedef struct D3D12_FEATURE_DATA_VIDEO_PROCESS_MAX_INPUT_STREAMS
    {
        UINT NodeIndex;                                             // input
        UINT MaxInputStreams;                                       // output
    } D3D12_FEATURE_DATA_VIDEO_PROCESS_MAX_INPUT_STREAMS;

    typedef struct D3D12_FEATURE_DATA_VIDEO_PROCESS_REFERENCE_INFO
    {
        UINT NodeIndex;                                             // input
        D3D12_VIDEO_PROCESS_DEINTERLACE_FLAGS DeinterlaceMode;      // input
        D3D12_VIDEO_PROCESS_FILTER_FLAGS Filters;                   // input
        D3D12_VIDEO_PROCESS_FEATURE_FLAGS FeatureSupport;           // input
        DXGI_RATIONAL InputFrameRate;                               // input
        DXGI_RATIONAL OutputFrameRate;                              // input 
        BOOL EnableAutoProcessing;                                  // input
        UINT PastFrames;                                            // output
        UINT FutureFrames;                                          // output
    } D3D12_FEATURE_DATA_VIDEO_PROCESS_REFERENCE_INFO;

    typedef struct D3D12_VIDEO_PROCESS_REFERENCE_SET
    {
        UINT NumPastFrames;
        ID3D12Resource **ppPastFrames;
        UINT *pPastSubresources;
        UINT NumFutureFrames;
        ID3D12Resource **ppFutureFrames;
        UINT *pFutureSubresources;
    } D3D12_VIDEO_PROCESS_REFERENCE_SET;

    typedef struct D3D12_VIDEO_PROCESS_TRANSFORM
    {
        D3D12_RECT SourceRectangle;
        D3D12_RECT DestinationRectangle;
        D3D12_VIDEO_PROCESS_ORIENTATION Orientation;
    } D3D12_VIDEO_PROCESS_TRANSFORM;

    typedef struct D3D12_VIDEO_PROCESS_INPUT_STREAM_RATE
    {
        UINT OutputIndex;
        UINT InputFrameOrField;
    } D3D12_VIDEO_PROCESS_INPUT_STREAM_RATE;

    typedef struct D3D12_VIDEO_PROCESS_INPUT_STREAM
    {
        ID3D12Resource *pTexture2D;
        UINT Subresource;
        D3D12_VIDEO_PROCESS_REFERENCE_SET ReferenceSet;
    } D3D12_VIDEO_PROCESS_INPUT_STREAM;
    
    #define	D3D12_VIDEO_PROCESS_STEREO_VIEWS	( 2 )

    typedef struct D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS
    {
        D3D12_VIDEO_PROCESS_INPUT_STREAM InputStream[D3D12_VIDEO_PROCESS_STEREO_VIEWS];
        D3D12_VIDEO_PROCESS_TRANSFORM Transform;
        D3D12_VIDEO_PROCESS_INPUT_STREAM_FLAGS Flags;
        D3D12_VIDEO_PROCESS_INPUT_STREAM_RATE RateInfo;
        INT FilterLevels[D3D12_VIDEO_PROCESS_MAX_FILTERS];
        D3D12_VIDEO_PROCESS_ALPHA_BLENDING AlphaBlending;
    } D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS;

    typedef struct D3D12_VIDEO_PROCESS_OUTPUT_STREAM
    {
        ID3D12Resource* pTexture2D;
        UINT Subresource;
    } D3D12_VIDEO_PROCESS_OUTPUT_STREAM;

    typedef struct D3D12_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS
    {
        D3D12_VIDEO_PROCESS_OUTPUT_STREAM OutputStream[D3D12_VIDEO_PROCESS_STEREO_VIEWS];
        D3D12_RECT TargetRectangle;
    } D3D12_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS;

};

extern "C"
{

//
// ID3D12VideoDevice
//

struct D3D12VideoDevice_Function_Table
{
    GraphicsUnknown_Function_Table ObjectFunction;
     
    HRESULT (D3DAPI *CheckFeatureSupport )( 
        ID3D12VideoDevice * This,
        D3D12_FEATURE_VIDEO FeatureVideo,
        _Inout_updates_bytes_(FeatureSupportDataSize)  void *pFeatureSupportData,
        UINT FeatureSupportDataSize);
    
    HRESULT (D3DAPI *CreateVideoDecoder )( 
        ID3D12VideoDevice * This,
        _In_  const D3D12_VIDEO_DECODER_DESC *pDesc,
        _In_  REFIID riid,
        _COM_Outptr_  void **ppVideoDecoder);
    
    HRESULT(D3DAPI *CreateVideoDecoderHeap)(
        ID3D12VideoDevice * This,
        _In_ const D3D12_VIDEO_DECODER_HEAP_DESC* pVideoDecoderHeapDesc,
        _In_ REFIID riid, // Expected: IID_ID3D12VideoDecoderHeap, 
        _COM_Outptr_  void** ppVideoDecoderHeap
    );

    HRESULT (D3DAPI *CreateVideoProcessor )(
            ID3D12VideoDevice * This,
            UINT NodeMask,
            _In_  const D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC *pOutputStreamDesc,
            UINT NumInputStreamDescs,
            _In_reads_(NumInputStreamDescs)  const D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC *pInputStreamDescs,
            _In_  REFIID riid,
            _COM_Outptr_  void **ppVideoProcessor);
    
};


#if DECLARE_UMD_EXPORTS_D3D12X

HRESULT D3DAPI D3D12VideoDevice_CheckFeatureSupport(
    ID3D12VideoDevice * This,
    D3D12_FEATURE_VIDEO FeatureVideo,
    _Inout_updates_bytes_(FeatureSupportDataSize)  void *pFeatureSupportData,
    UINT FeatureSupportDataSize);

HRESULT D3DAPI D3D12VideoDevice_CreateVideoDecoder(
    ID3D12VideoDevice * This,
    _In_  const D3D12_VIDEO_DECODER_DESC *pDesc,
    _In_  REFIID riid,
    _COM_Outptr_  void **ppVideoDecoder);

HRESULT D3DAPI D3D12VideoDevice_CreateVideoDecoderHeap(
    ID3D12VideoDevice * This,
    _In_ const D3D12_VIDEO_DECODER_HEAP_DESC* pVideoDecoderHeapDesc,
    _In_ REFIID riid, // Expected: IID_ID3D12VideoDecoderHeap, 
    _COM_Outptr_  void** ppVideoDecoderHeap);

HRESULT D3DAPI D3D12VideoDevice_CreateVideoProcessor(
    ID3D12VideoDevice * This,
    UINT NodeMask,
    _In_  const D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC *pOutputStreamDesc,
    UINT NumInputStreamDescs,
    _In_reads_(NumInputStreamDescs)  const D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC *pInputStreamDescs,
    _In_  REFIID riid,
    _COM_Outptr_  void **ppVideoProcessor);


ULONG D3DAPI D3D12VideoDevice_AddRef(
    _Inout_ IGraphicsUnknown* pUnknown);

ULONG D3DAPI D3D12VideoDevice_Release(
    _Inout_ IGraphicsUnknown* pUnknown);

HRESULT D3DAPI D3D12VideoDevice_QueryInterface(
    _Inout_ IGraphicsUnknown* pUnknown,
    _In_ REFIID riid,
    _Out_ void** ppObject);

D3DCONST D3D12VideoDevice_Function_Table D3D__D3d12VideoDevice_Function_Table = 
{
    {
        D3D12VideoDevice_QueryInterface,
        D3D12VideoDevice_AddRef,
        D3D12VideoDevice_Release,
    },
    D3D12VideoDevice_CheckFeatureSupport,
    D3D12VideoDevice_CreateVideoDecoder,
    D3D12VideoDevice_CreateVideoDecoderHeap,
    D3D12VideoDevice_CreateVideoProcessor,
};

#endif
};


D3DINTERFACE(ID3D12VideoDevice, 1F052807,0B46,4ACC,8A,89,36,4F,79,37,18,A4) : public IGraphicsUnknown
{
public:

    D3DINLINE D3D12VideoDevice_Function_Table* DeviceFunction()
    {
        return (D3D12VideoDevice_Function_Table*)m_pFunction;
    }
    
    D3DINLINE HRESULT D3DAPI CheckFeatureSupport( 
        D3D12_FEATURE_VIDEO FeatureVideo,
        _Inout_updates_bytes_(FeatureSupportDataSize)  void *pFeatureSupportData,
        UINT FeatureSupportDataSize)
    {
        return DeviceFunction()->CheckFeatureSupport(this, FeatureVideo, pFeatureSupportData, FeatureSupportDataSize);
    }
    
    D3DINLINE HRESULT D3DAPI CreateVideoDecoder( 
        _In_  const D3D12_VIDEO_DECODER_DESC *pDesc,
        _In_  REFIID riid,
        _COM_Outptr_  void **ppVideoDecoder)
    {
        return DeviceFunction()->CreateVideoDecoder(this, pDesc, riid, ppVideoDecoder);
    }
   
    D3DINLINE HRESULT D3DAPI CreateVideoDecoderHeap(
        _In_  const D3D12_VIDEO_DECODER_HEAP_DESC* pDesc,
        _In_  REFIID riid,  // Expected: IID_ID3D12VideoDecoderHeap, 
        _COM_Outptr_  void **ppVideoDecoderHeap)
    {
        return DeviceFunction()->CreateVideoDecoderHeap(this, pDesc, riid, ppVideoDecoderHeap);
    }
    
    D3DINLINE HRESULT D3DAPI CreateVideoProcessor( 
        UINT NodeMask,
        _In_  const D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC *pOutputStreamDesc,
        UINT NumInputStreamDescs,
        _In_reads_(NumInputStreamDescs)  const D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC *pInputStreamDescs,
        _In_  REFIID riid,
        _COM_Outptr_  void **ppVideoProcessor)
    {
        return DeviceFunction()->CreateVideoProcessor(this, NodeMask, pOutputStreamDesc, NumInputStreamDescs, pInputStreamDescs, riid, ppVideoProcessor);
    }
};


extern "C"
{

//
// ID3D12VideoDecode
//

struct D3D12VideoDecoder_Function_Table
{
    D3D12Pageable_Function_Table ObjectFunction;
   
    D3D12_VIDEO_DECODER_DESC ( D3DAPI *GetDesc )( ID3D12VideoDecoder * pThis);

};


#if DECLARE_UMD_EXPORTS_D3D12X

D3D12_VIDEO_DECODER_DESC D3DAPI D3D12VideoDecoder_GetDesc( ID3D12VideoDecoder * pThis);

ULONG D3DAPI D3d12VideoDecoder_Release(_Inout_ IGraphicsUnknown* pUnknown);

D3DCONST D3D12VideoDecoder_Function_Table D3D__D3d12VideoDecoder_Function_Table = 
{
    {
        {
            {
                {
                    D3d12Object_QueryInterface,
                    D3d12DeviceChild_AddRef,
                    D3d12VideoDecoder_Release,
                },
                D3d12Object_GetPrivateData,
                D3d12Object_SetPrivateData,
                D3d12Object_SetPrivateDataInterface,
                D3d12Object_SetName,
            },

            D3d12DeviceChild_GetDevice,
        },
    },
    D3D12VideoDecoder_GetDesc,
};
#endif

// external c
};



D3DINTERFACE(ID3D12VideoDecoder, C59B6BDC,7720,4074,A1,36,17,A1,56,03,74,70) : public ID3D12Pageable
{
public:

    D3DINLINE D3D12VideoDecoder_Function_Table* DecoderFunction()
    {
        return (D3D12VideoDecoder_Function_Table*)m_pFunction;
    }

    D3DINLINE D3D12_VIDEO_DECODER_DESC D3DAPI GetDesc()
    {
        return DecoderFunction()->GetDesc(this);
    }
};

 

extern "C"
{
    
    typedef 
    enum D3D12_VIDEO_DECODE_TIER
        {
            D3D12_VIDEO_DECODE_TIER_NOT_SUPPORTED   = 0,
            D3D12_VIDEO_DECODE_TIER_1   = 1,
            D3D12_VIDEO_DECODE_TIER_2   = 2,
            D3D12_VIDEO_DECODE_TIER_3   = 3,
        }   D3D12_VIDEO_DECODE_TIER;
    
    typedef 
    enum D3D12_VIDEO_DECODE_SUPPORT_FLAGS
        {
            D3D12_VIDEO_DECODE_SUPPORT_FLAG_NONE    = 0,
            D3D12_VIDEO_DECODE_SUPPORT_FLAG_SUPPORTED   = 0x1,
        }   D3D12_VIDEO_DECODE_SUPPORT_FLAGS;
    
    DEFINE_ENUM_FLAG_OPERATORS(D3D12_VIDEO_DECODE_SUPPORT_FLAGS );
    typedef 
    enum D3D12_VIDEO_DECODE_CONFIGURATION_FLAGS
        {
            D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_NONE  = 0,
            D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_HEIGHT_ALIGNMENT_MULTIPLE_32_REQUIRED = 0x1,
            D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_POST_PROCESSING_SUPPORTED = 0x2,
            D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_REFERENCE_ONLY_ALLOCATIONS_REQUIRED = 0x4,
            D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_ALLOW_RESOLUTION_CHANGE_ON_NON_KEY_FRAME = 0x8,
    }   D3D12_VIDEO_DECODE_CONFIGURATION_FLAGS;
    
    DEFINE_ENUM_FLAG_OPERATORS(D3D12_VIDEO_DECODE_CONFIGURATION_FLAGS );
    typedef 
    enum D3D12_VIDEO_DECODE_STATUS
        {
            D3D12_VIDEO_DECODE_STATUS_OK    = 0,
            D3D12_VIDEO_DECODE_STATUS_CONTINUE  = 1,
            D3D12_VIDEO_DECODE_STATUS_CONTINUE_SKIP_DISPLAY = 2,
            D3D12_VIDEO_DECODE_STATUS_RESTART   = 3,
        }   D3D12_VIDEO_DECODE_STATUS;
    
    typedef 
    enum D3D12_VIDEO_DECODE_ARGUMENT_TYPE
        {
            D3D12_VIDEO_DECODE_ARGUMENT_TYPE_PICTURE_PARAMETERS = 0,
            D3D12_VIDEO_DECODE_ARGUMENT_TYPE_INVERSE_QUANTIZATION_MATRIX    = 1,
            D3D12_VIDEO_DECODE_ARGUMENT_TYPE_SLICE_CONTROL  = 2,
            D3D12_VIDEO_DECODE_ARGUMENT_TYPE_MAX_VALID  = 3
        }   D3D12_VIDEO_DECODE_ARGUMENT_TYPE;
    
    typedef struct D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT
        {
            UINT NodeIndex;
            D3D12_VIDEO_DECODE_CONFIGURATION Configuration;
            UINT Width;
            UINT Height;
            DXGI_FORMAT DecodeFormat;
            DXGI_RATIONAL FrameRate;
            UINT BitRate;
            D3D12_VIDEO_DECODE_SUPPORT_FLAGS SupportFlags;
            D3D12_VIDEO_DECODE_CONFIGURATION_FLAGS ConfigurationFlags;
            D3D12_VIDEO_DECODE_TIER DecodeTier;
        }   D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT;
    
    typedef struct D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILES
        {
            UINT NodeIndex;
            UINT ProfileCount;
            _Field_size_full_(ProfileCount)  GUID *pProfiles;
        }   D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILES;

    typedef struct D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILE_COUNT
        {
            UINT NodeIndex;                                             // input
            UINT ProfileCount;                                          // output
        } D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILE_COUNT;

    typedef struct D3D12_FEATURE_DATA_VIDEO_DECODE_FORMAT_COUNT
        {
            UINT NodeIndex;                                                             // input
            D3D12_VIDEO_DECODE_CONFIGURATION Configuration;                             // input
            UINT FormatCount;                                                           // output
        } D3D12_FEATURE_DATA_VIDEO_DECODE_FORMAT_COUNT;

    typedef struct D3D12_FEATURE_DATA_VIDEO_DECODE_FORMATS
        {
            UINT NodeIndex;
            D3D12_VIDEO_DECODE_CONFIGURATION Configuration;
            UINT FormatCount;
            _Field_size_full_(FormatCount)  DXGI_FORMAT *pOutputFormats;
        }   D3D12_FEATURE_DATA_VIDEO_DECODE_FORMATS;

    typedef struct D3D12_FEATURE_DATA_VIDEO_ARCHITECTURE
        {
            BOOL IOCoherent;
        } D3D12_FEATURE_DATA_VIDEO_ARCHITECTURE;

    typedef 
    enum D3D12_VIDEO_DECODE_CONVERSION_SUPPORT_FLAGS
        {
            D3D12_VIDEO_DECODE_CONVERSION_SUPPORT_FLAG_NONE = 0,
            D3D12_VIDEO_DECODE_CONVERSION_SUPPORT_FLAG_SUPPORTED    = 0x1,
        }   D3D12_VIDEO_DECODE_CONVERSION_SUPPORT_FLAGS;
    
    DEFINE_ENUM_FLAG_OPERATORS(D3D12_VIDEO_DECODE_CONVERSION_SUPPORT_FLAGS );

    typedef struct D3D12_FEATURE_DATA_VIDEO_DECODE_CONVERSION_SUPPORT
        {
            UINT NodeIndex;                                             // input
            D3D12_VIDEO_DECODE_CONFIGURATION Configuration;             // input
            D3D12_VIDEO_SAMPLE DecodeSample;                            // input
            D3D12_VIDEO_FORMAT OutputFormat;                            // input
            DXGI_RATIONAL FrameRate;                                    // input
            UINT BitRate;                                               // input
            D3D12_VIDEO_DECODE_CONVERSION_SUPPORT_FLAGS SupportFlags;   // output
            D3D12_VIDEO_SCALE_SUPPORT ScaleSupport;                     // output
        } D3D12_FEATURE_DATA_VIDEO_DECODE_CONVERSION_SUPPORT;


    typedef struct D3D12_FEATURE_DATA_VIDEO_DECODER_HEAP_SIZE
        {
            D3D12_VIDEO_DECODER_HEAP_DESC VideoDecoderHeapDesc;         // input
            UINT64 MemoryPoolL0Size;                                    // output
            UINT64 MemoryPoolL1Size;                                    // output
        } D3D12_FEATURE_DATA_VIDEO_DECODER_HEAP_SIZE;

    typedef struct D3D12_QUERY_DATA_VIDEO_DECODE_STATISTICS
        {
            UINT64 Status;                      // see D3D12_VIDEO_DECODE_STATUS 
            UINT64 NumMacroblocksAffected;
            DXGI_RATIONAL FrameRate;
            UINT BitRate;
        } D3D12_QUERY_DATA_VIDEO_DECODE_STATISTICS;

    typedef struct D3D12_FEATURE_DATA_VIDEO_DECODE_BITSTREAM_ENCRYPTION_SCHEMES
        {
            UINT NodeIndex;
            GUID DecodeProfile;
            UINT EncryptionSchemeCount;
            _Field_size_full_(EncryptionSchemeCount)  D3D12_BITSTREAM_ENCRYPTION_TYPE *pEncryptionSchemes;
        }   D3D12_FEATURE_DATA_VIDEO_DECODE_BITSTREAM_ENCRYPTION_SCHEMES;
    
    typedef struct D3D12_VIDEO_DECODE_SUB_SAMPLE_MAPPING_BLOCK
        {
        UINT ClearSize;
        UINT EncryptedSize;
        }   D3D12_VIDEO_DECODE_SUB_SAMPLE_MAPPING_BLOCK;
    
    typedef struct D3D12_VIDEO_DECODE_FRAME_ARGUMENT
        {
            D3D12_VIDEO_DECODE_ARGUMENT_TYPE Type;
            _Field_size_bytes_full_(ArgumentSize)  void *pData;
            UINT Size;
        }   D3D12_VIDEO_DECODE_FRAME_ARGUMENT;
    
    typedef struct D3D12_VIDEO_DECODE_REFERENCE_FRAMES
        {
            _Field_size_full_(NumTexture2Ds)  ID3D12Resource **ppTexture2Ds;
            _Field_size_full_(NumTexture2Ds)  UINT *pSubresources;
            UINT NumTexture2Ds;
        }   D3D12_VIDEO_DECODE_REFERENCE_FRAMES;
    
    typedef struct D3D12_VIDEO_DECODE_COMPRESSED_BITSTREAM
        {
            ID3D12Resource *pBuffer;
            UINT64 Offset;
            UINT64 Size;
        }   D3D12_VIDEO_DECODE_COMPRESSED_BITSTREAM;
    
    typedef struct D3D12_VIDEO_DECODE_CONVERSION_ARGUMENTS
        {
            BOOL Enable;
            ID3D12Resource *pReferenceTexture2D;
            UINT ReferenceSubresource;
            DXGI_COLOR_SPACE_TYPE OutputColorSpace;
            DXGI_COLOR_SPACE_TYPE DecodeColorSpace;
        }   D3D12_VIDEO_DECODE_CONVERSION_ARGUMENTS;

   
#define	D3D12_VIDEO_DECODE_MAX_ARGUMENTS	( 10 )
    
    typedef struct D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS
    {
        UINT NumFrameArguments;
        D3D12_VIDEO_DECODE_FRAME_ARGUMENT FrameArguments[D3D12_VIDEO_DECODE_MAX_ARGUMENTS];
        D3D12_VIDEO_DECODE_REFERENCE_FRAMES ReferenceFrames;
        D3D12_VIDEO_DECODE_COMPRESSED_BITSTREAM CompressedBitstream;
        ID3D12VideoDecoderHeap *pHeap;
    } D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS;

    typedef struct D3D12_VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS
    {
        ID3D12Resource* pOutputTexture2D;
        UINT OutputSubresource;
        D3D12_VIDEO_DECODE_CONVERSION_ARGUMENTS ConversionArguments;
    } D3D12_VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS;

    

 // external_c 
}

extern "C"
{

    //
    // ID3D12VideoProcessor
    //

    struct D3D12VideoProcessor_Function_Table
    {
        D3D12Pageable_Function_Table ObjectFunction;

        UINT(D3DAPI *GetNodeMask)(ID3D12VideoProcessor * pThis);
        UINT(D3DAPI *GetNumInputStreamDescs)(ID3D12VideoProcessor * pThis);
        HRESULT(D3DAPI *GetInputStreamDescs)(ID3D12VideoProcessor * pThis, UINT NumInputStreamDescs, D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC *pInputStreamDescs);
        D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC (D3DAPI *GetOutputStreamDesc)(ID3D12VideoProcessor * pThis);
    };


#if DECLARE_UMD_EXPORTS_D3D12X

    ULONG D3DAPI D3D12VideoProcessor_Release(IGraphicsUnknown *pVideoProcessor);
    D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC D3DAPI D3D12VideoProcessor_GetOutputStreamDesc(ID3D12VideoProcessor * pThis);
    UINT D3DAPI D3D12VideoProcessor_GetNodeMask(ID3D12VideoProcessor * pThis);
    UINT D3DAPI D3D12VideoProcessor_GetNumInputStreamDescs(ID3D12VideoProcessor * pThis);
    HRESULT D3DAPI D3D12VideoProcessor_GetInputStreamDescs(
        ID3D12VideoProcessor * pThis,
        UINT NumInputStreamDescs,
        D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC *pInputStreamDescs);

    D3DCONST D3D12VideoProcessor_Function_Table D3D__D3D12VideoProcessor_Function_Table =
    {
        {
            {
                {
                    {
                        D3d12Object_QueryInterface,
                        D3d12DeviceChild_AddRef,
                        D3D12VideoProcessor_Release,
                    },
                    D3d12Object_GetPrivateData,
                    D3d12Object_SetPrivateData,
                    D3d12Object_SetPrivateDataInterface,
                    D3d12Object_SetName,
                },

                D3d12DeviceChild_GetDevice,
            },
        },
        D3D12VideoProcessor_GetNodeMask,
        D3D12VideoProcessor_GetNumInputStreamDescs,
        D3D12VideoProcessor_GetInputStreamDescs,
        D3D12VideoProcessor_GetOutputStreamDesc
    };
#endif

    // external c
};


D3DINTERFACE(ID3D12VideoProcessor, 304FDB32, bede, 410a, 85, 45, 94, 3a, c6, a4, 61, 38) : public ID3D12Pageable
{
public:

    D3DINLINE D3D12VideoProcessor_Function_Table* VideoProcessorFunction()
    {
        return (D3D12VideoProcessor_Function_Table*)m_pFunction;
    }

    D3DINLINE UINT GetNodeMask()
    {
        return VideoProcessorFunction()->GetNodeMask(this);
    }

    D3DINLINE UINT GetNumInputStreamDescs()
    {
        return VideoProcessorFunction()->GetNodeMask(this);
    }

    D3DINLINE HRESULT GetInputStreamDescs(UINT NumInputStreamDescs, D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC *pInputStreamDescs)
    {
        return VideoProcessorFunction()->GetInputStreamDescs(this, NumInputStreamDescs, pInputStreamDescs);
    }

    D3DINLINE D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC GetOutputStreamDesc()
    {
        return VideoProcessorFunction()->GetOutputStreamDesc(this);
    }
};

extern "C"
{

// ID3D12VideoDecodeCommandList
//

struct D3D12VideoDecodeCommandList_Function_Table
{
    D3D12CommandList_Function_Table ObjectFunction;
        
    HRESULT (D3DAPI *Close )(
        _In_ ID3D12VideoDecodeCommandList * pThis);
    
    HRESULT (D3DAPI *Reset )(
        _In_  ID3D12VideoDecodeCommandList * pThis,
        _In_  ID3D12CommandAllocator *pAllocator);
    
    void (D3DAPI *ClearState )(_In_  ID3D12VideoDecodeCommandList * pThis);
    
    void (D3DAPI *ResourceBarrier )( 
         _In_  ID3D12VideoDecodeCommandList * pThis,
         _In_  UINT NumBarriers,
        _In_reads_(NumBarriers)  const D3D12_RESOURCE_BARRIER *pBarriers);
    
    void (D3DAPI *DiscardResource )( 
        _In_  ID3D12VideoDecodeCommandList * pThis,
         _In_  ID3D12Resource *pResource,
        _In_opt_  const D3D12_DISCARD_REGION *pRegion);
    
    void (D3DAPI *BeginQuery )( 
        _In_  ID3D12VideoDecodeCommandList * pThis,
        _In_  ID3D12QueryHeap *pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT Index);
    
    void (D3DAPI *EndQuery )( 
        _In_  ID3D12VideoDecodeCommandList * pThis,
        _In_  ID3D12QueryHeap *pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT Index);
    
    void (D3DAPI *ResolveQueryData )( 
        _In_  ID3D12VideoDecodeCommandList * pThis,
        _In_  ID3D12QueryHeap *pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT StartIndex,
        _In_  UINT NumQueries,
        _In_  ID3D12Resource *pDestinationBuffer,
        _In_  UINT64 AlignedDestinationBufferOffset);
    
    void (D3DAPI *SetPredication )( 
        _In_  ID3D12VideoDecodeCommandList * pThis,
         _In_opt_  ID3D12Resource *pBuffer,
        _In_  UINT64 AlignedBufferOffset,
        _In_  D3D12_PREDICATION_OP Operation);
    
    void (D3DAPI *SetMarker )( 
        _In_  ID3D12VideoDecodeCommandList * pThis,
         UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void *pData,
        UINT Size);
    
    void (D3DAPI *BeginEvent )( 
        _In_  ID3D12VideoDecodeCommandList * pThis,
         UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void *pData,
        UINT Size);
    
    void (D3DAPI *EndEvent )(_In_ ID3D12VideoDecodeCommandList * pThis);
    
    void (D3DAPI *DecodeFrame )( 
        _In_  ID3D12VideoDecodeCommandList * pThis,
        _In_  ID3D12VideoDecoder *pDecoder,
        _In_  const D3D12_VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS *pOutputArguments,
        _In_  const D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS *pInputArguments);
    
    void (D3DAPI *WriteBufferImmediate)(
        _In_  ID3D12VideoDecodeCommandList * pThis,
        UINT Count,
        _In_reads_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER *pParams,
        _In_reads_opt_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_MODE *pModes);
};

#if DECLARE_UMD_EXPORTS_D3D12X


HRESULT D3DAPI D3D12VideoDecodeCommandList_Close(
    _In_ ID3D12VideoDecodeCommandList * pThis);

HRESULT D3DAPI D3D12VideoDecodeCommandList_Reset(
    _In_  ID3D12VideoDecodeCommandList * pThis,
    _In_  ID3D12CommandAllocator *pAllocator);

void D3DAPI D3D12VideoDecodeCommandList_ClearState(_In_  ID3D12VideoDecodeCommandList * pThis);

void D3DAPI D3D12VideoDecodeCommandList_ResourceBarrier( 
     _In_  ID3D12VideoDecodeCommandList * pThis,
     _In_  UINT NumBarriers,
    _In_reads_(NumBarriers)  const D3D12_RESOURCE_BARRIER *pBarriers);

void D3DAPI D3D12VideoDecodeCommandList_DiscardResource( 
    _In_  ID3D12VideoDecodeCommandList * pThis,
     _In_  ID3D12Resource *pResource,
    _In_opt_  const D3D12_DISCARD_REGION *pRegion);

void D3DAPI D3D12VideoDecodeCommandList_BeginQuery( 
    _In_  ID3D12VideoDecodeCommandList * pThis,
    _In_  ID3D12QueryHeap *pQueryHeap,
    _In_  D3D12_QUERY_TYPE Type,
    _In_  UINT Index);

void D3DAPI D3D12VideoDecodeCommandList_EndQuery( 
    _In_  ID3D12VideoDecodeCommandList * pThis,
    _In_  ID3D12QueryHeap *pQueryHeap,
    _In_  D3D12_QUERY_TYPE Type,
    _In_  UINT Index);

void D3DAPI D3D12VideoDecodeCommandList_ResolveQueryData( 
    _In_  ID3D12VideoDecodeCommandList * pThis,
    _In_  ID3D12QueryHeap *pQueryHeap,
    _In_  D3D12_QUERY_TYPE Type,
    _In_  UINT StartIndex,
    _In_  UINT NumQueries,
    _In_  ID3D12Resource *pDestinationBuffer,
    _In_  UINT64 AlignedDestinationBufferOffset);

void D3DAPI D3D12VideoDecodeCommandList_SetPredication( 
    _In_  ID3D12VideoDecodeCommandList * pThis,
     _In_opt_  ID3D12Resource *pBuffer,
    _In_  UINT64 AlignedBufferOffset,
    _In_  D3D12_PREDICATION_OP Operation);

void D3DAPI D3D12VideoDecodeCommandList_SetMarker( 
    _In_  ID3D12VideoDecodeCommandList * pThis,
     UINT Metadata,
    _In_reads_bytes_opt_(Size)  const void *pData,
    UINT Size);

void D3DAPI D3D12VideoDecodeCommandList_BeginEvent( 
    _In_  ID3D12VideoDecodeCommandList * pThis,
     UINT Metadata,
    _In_reads_bytes_opt_(Size)  const void *pData,
    UINT Size);

void D3DAPI D3D12VideoDecodeCommandList_EndEvent(_In_ ID3D12VideoDecodeCommandList * pThis);

void D3DAPI D3D12VideoDecodeCommandList_DecodeFrame( 
    _In_  ID3D12VideoDecodeCommandList * pThis,
    _In_  ID3D12VideoDecoder *pDecoder,
    _In_  const D3D12_VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS *pOutputArguments,
    _In_  const D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS *pInputArguments);

void D3DAPI D3D12VideoDecodeCommandList_WriteBufferImmediate(
    _In_  ID3D12VideoDecodeCommandList * pThis,
    UINT count,
    _In_reads_(count)   const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER *pParams,
    _In_reads_opt_(count)  const D3D12_WRITEBUFFERIMMEDIATE_MODE *pModes);


ULONG D3DAPI D3D12VideoDecodeCommandList_Release(
    _Inout_ IGraphicsUnknown* pCommandList);

D3DCONST D3D12VideoDecodeCommandList_Function_Table D3D__D3D12VideoDecodeCommandList_Function_Table = 
{
    {
        { 
            { 
                { 
                    D3d12Object_QueryInterface,
                    D3d12DeviceChild_AddRef,
                    D3D12VideoDecodeCommandList_Release,
                },

                D3d12Object_GetPrivateData,
                D3d12Object_SetPrivateData,
                D3d12Object_SetPrivateDataInterface,
                D3d12Object_SetName,
            },

            D3d12DeviceChild_GetDevice,
        },

        D3d12CommandList_GetType,
    },

    D3D12VideoDecodeCommandList_Close,
    D3D12VideoDecodeCommandList_Reset,
    D3D12VideoDecodeCommandList_ClearState,
    D3D12VideoDecodeCommandList_ResourceBarrier,
    D3D12VideoDecodeCommandList_DiscardResource,
    D3D12VideoDecodeCommandList_BeginQuery,
    D3D12VideoDecodeCommandList_EndQuery, 
    D3D12VideoDecodeCommandList_ResolveQueryData,
    D3D12VideoDecodeCommandList_SetPredication,
    D3D12VideoDecodeCommandList_SetMarker,
    D3D12VideoDecodeCommandList_BeginEvent,
    D3D12VideoDecodeCommandList_EndEvent,
    D3D12VideoDecodeCommandList_DecodeFrame,
    D3D12VideoDecodeCommandList_WriteBufferImmediate,
    
};
#endif


// external c
};


D3DINTERFACE(ID3D12VideoDecodeCommandList, 3B60536E, AD29, 4E64, A2, 69, F8, 53, 83, 7E, 5E, 53) : public ID3D12CommandList
{
public:

    D3DINLINE D3D12VideoDecodeCommandList_Function_Table* CommandListFunction()
    {
        return (D3D12VideoDecodeCommandList_Function_Table*)m_pFunction;
    }


    D3DINLINE HRESULT D3DAPI Close()
    {
        return CommandListFunction()->Close(this);
    }

    D3DINLINE HRESULT D3DAPI Reset(
        _In_  ID3D12CommandAllocator *pAllocator)
    {
        return CommandListFunction()->Reset(this, pAllocator);
    }

    D3DINLINE void D3DAPI ClearState()
    {
        return CommandListFunction()->ClearState(this);
    }

    D3DINLINE void D3DAPI ResourceBarrier(
         _In_  UINT NumBarriers,
        _In_reads_(NumBarriers)  const D3D12_RESOURCE_BARRIER *pBarriers)
    {
        return CommandListFunction()->ResourceBarrier(this, NumBarriers, pBarriers);
    }

    D3DINLINE void D3DAPI DiscardResource(
         _In_  ID3D12Resource *pResource,
        _In_opt_  const D3D12_DISCARD_REGION *pRegion)
    {
        return CommandListFunction()->DiscardResource(this, pResource, pRegion);
    }

    D3DINLINE void D3DAPI BeginQuery(
         _In_  ID3D12QueryHeap *pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT Index)
    {
        return CommandListFunction()->BeginQuery(this, pQueryHeap, Type, Index);
    }

    D3DINLINE void D3DAPI EndQuery(
         _In_  ID3D12QueryHeap *pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT Index)
    {
        return CommandListFunction()->EndQuery(this, pQueryHeap, Type, Index);
    }

    D3DINLINE void D3DAPI ResolveQueryData(
         _In_  ID3D12QueryHeap *pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT StartIndex,
        _In_  UINT NumQueries,
        _In_  ID3D12Resource *pDestinationBuffer,
        _In_  UINT64 AlignedDestinationBufferOffset)
    {
        return CommandListFunction()->ResolveQueryData(this, pQueryHeap, Type, StartIndex, NumQueries, pDestinationBuffer, AlignedDestinationBufferOffset);
    }

    D3DINLINE void D3DAPI SetPredication(
         _In_opt_  ID3D12Resource *pBuffer,
        _In_  UINT64 AlignedBufferOffset,
        _In_  D3D12_PREDICATION_OP Operation)
    {
        return CommandListFunction()->SetPredication(this, pBuffer, AlignedBufferOffset, Operation);
    }

    D3DINLINE void D3DAPI SetMarker(
         UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void *pData,
        UINT Size)
    {
        return CommandListFunction()->SetMarker(this, Metadata, pData, Size);
    }

    D3DINLINE void D3DAPI BeginEvent(
         UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void *pData,
        UINT Size)
    {
        return CommandListFunction()->BeginEvent(this, Metadata, pData, Size);
    }

    D3DINLINE void D3DAPI EndEvent()
    {
        return CommandListFunction()->EndEvent(this);
    }

    D3DINLINE void D3DAPI DecodeFrame(
        _In_  ID3D12VideoDecoder *pDecoder,
        _In_  const D3D12_VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS *pOutputArguments,
        _In_  const D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS *pInputArguments)
    {
        return CommandListFunction()->DecodeFrame(this, pDecoder, pOutputArguments, pInputArguments);
    }

    D3DINLINE void D3DAPI WriteBufferImmediate(
        UINT Count,
        _In_reads_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER *pParams,
        _In_reads_opt_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_MODE *pModes)
    {
        return CommandListFunction()->WriteBufferImmediate(this, Count, pParams, pModes);
    }
};


extern "C"
{

    // ID3D12VideoProcessCommandList
    //

    struct D3D12VideoProcessCommandList_Function_Table
    {
        D3D12CommandList_Function_Table ObjectFunction;

        HRESULT(D3DAPI *Close)(
            _In_ ID3D12VideoProcessCommandList * pThis);

        HRESULT(D3DAPI *Reset)(
            _In_  ID3D12VideoProcessCommandList * pThis,
            _In_  ID3D12CommandAllocator *pAllocator);

        void (D3DAPI *ClearState)(_In_  ID3D12VideoProcessCommandList * pThis);

        void (D3DAPI *ResourceBarrier)(
            _In_  ID3D12VideoProcessCommandList * pThis,
            _In_  UINT NumBarriers,
            _In_reads_(NumBarriers)  const D3D12_RESOURCE_BARRIER *pBarriers);

        void (D3DAPI *DiscardResource)(
            _In_  ID3D12VideoProcessCommandList * pThis,
            _In_  ID3D12Resource *pResource,
            _In_opt_  const D3D12_DISCARD_REGION *pRegion);

        void (D3DAPI *BeginQuery)(
            _In_  ID3D12VideoProcessCommandList * pThis,
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT Index);

        void (D3DAPI *EndQuery)(
            _In_  ID3D12VideoProcessCommandList * pThis,
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT Index);

        void (D3DAPI *ResolveQueryData)(
            _In_  ID3D12VideoProcessCommandList * pThis,
            _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT StartIndex,
            _In_  UINT NumQueries,
            _In_  ID3D12Resource *pDestinationBuffer,
            _In_  UINT64 AlignedDestinationBufferOffset);

        void (D3DAPI *SetPredication)(
            _In_  ID3D12VideoProcessCommandList * pThis,
            _In_opt_  ID3D12Resource *pBuffer,
            _In_  UINT64 AlignedBufferOffset,
            _In_  D3D12_PREDICATION_OP Operation);

        void (D3DAPI *SetMarker)(
            _In_  ID3D12VideoProcessCommandList * pThis,
            UINT Metadata,
            _In_reads_bytes_opt_(Size)  const void *pData,
            UINT Size);

        void (D3DAPI *BeginEvent)(
            _In_  ID3D12VideoProcessCommandList * pThis,
            UINT Metadata,
            _In_reads_bytes_opt_(Size)  const void *pData,
            UINT Size);

        void (D3DAPI *EndEvent)(_In_ ID3D12VideoProcessCommandList * pThis);

        void (D3DAPI *ProcessFrames)(
            _In_  ID3D12VideoProcessCommandList * pThis,
            _In_  ID3D12VideoProcessor *pVideoProcessor,
            _In_  const D3D12_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS *pOutputArguments,
            UINT NumInputStreams,
            _In_  const D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS *pInputArguments);

        void (D3DAPI *WriteBufferImmediate)(
            _In_  ID3D12VideoProcessCommandList * pThis,
            UINT Count,
            _In_reads_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER *pParams,
            _In_reads_opt_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_MODE *pModes);
    };

#if DECLARE_UMD_EXPORTS_D3D12X


    HRESULT D3DAPI D3D12VideoProcessCommandList_Close(
        _In_ ID3D12VideoProcessCommandList * pThis);

    HRESULT D3DAPI D3D12VideoProcessCommandList_Reset(
        _In_  ID3D12VideoProcessCommandList * pThis,
        _In_  ID3D12CommandAllocator *pAllocator);

    void D3DAPI D3D12VideoProcessCommandList_ClearState(_In_  ID3D12VideoProcessCommandList * pThis);

    void D3DAPI D3D12VideoProcessCommandList_ResourceBarrier(
        _In_  ID3D12VideoProcessCommandList * pThis,
        _In_  UINT NumBarriers,
        _In_reads_(NumBarriers)  const D3D12_RESOURCE_BARRIER *pBarriers);

    void D3DAPI D3D12VideoProcessCommandList_DiscardResource(
        _In_  ID3D12VideoProcessCommandList * pThis,
        _In_  ID3D12Resource *pResource,
        _In_opt_  const D3D12_DISCARD_REGION *pRegion);

    void D3DAPI D3D12VideoProcessCommandList_BeginQuery(
        _In_  ID3D12VideoProcessCommandList * pThis,
        _In_  ID3D12QueryHeap *pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT Index);

    void D3DAPI D3D12VideoProcessCommandList_EndQuery(
        _In_  ID3D12VideoProcessCommandList * pThis,
        _In_  ID3D12QueryHeap *pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT Index);

    void D3DAPI D3D12VideoProcessCommandList_ResolveQueryData(
        _In_  ID3D12VideoProcessCommandList * pThis,
        _In_  ID3D12QueryHeap *pQueryHeap,
        _In_  D3D12_QUERY_TYPE Type,
        _In_  UINT StartIndex,
        _In_  UINT NumQueries,
        _In_  ID3D12Resource *pDestinationBuffer,
        _In_  UINT64 AlignedDestinationBufferOffset);

    void D3DAPI D3D12VideoProcessCommandList_SetPredication(
        _In_  ID3D12VideoProcessCommandList * pThis,
        _In_opt_  ID3D12Resource *pBuffer,
        _In_  UINT64 AlignedBufferOffset,
        _In_  D3D12_PREDICATION_OP Operation);

    void D3DAPI D3D12VideoProcessCommandList_SetMarker(
        _In_  ID3D12VideoProcessCommandList * pThis,
        UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void *pData,
        UINT Size);

    void D3DAPI D3D12VideoProcessCommandList_BeginEvent(
        _In_  ID3D12VideoProcessCommandList * pThis,
        UINT Metadata,
        _In_reads_bytes_opt_(Size)  const void *pData,
        UINT Size);

    void D3DAPI D3D12VideoProcessCommandList_EndEvent(_In_ ID3D12VideoProcessCommandList * pThis);

    void D3DAPI D3D12VideoProcessCommandList_ProcessFrames(
        _In_  ID3D12VideoProcessCommandList * pThis,
        _In_  ID3D12VideoProcessor *pVideoProcessor,
        _In_  const D3D12_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS *pOutputArguments,
        UINT NumInputStreams,
        _In_  const D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS *pInputArguments);

    void D3DAPI D3D12VideoProcessCommandList_WriteBufferImmediate(
        _In_  ID3D12VideoProcessCommandList * pThis,
        UINT count,
        _In_reads_(count)   const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER *pParams,
        _In_reads_opt_(count)  const D3D12_WRITEBUFFERIMMEDIATE_MODE *pModes);


    ULONG D3DAPI D3D12VideoProcessCommandList_Release(
        _Inout_ IGraphicsUnknown* pCommandList);

    D3DCONST D3D12VideoProcessCommandList_Function_Table D3D__D3D12VideoProcessCommandList_Function_Table =
    {
        {
            {
                {
                    {
                        D3d12Object_QueryInterface,
                        D3d12DeviceChild_AddRef,
                        D3D12VideoProcessCommandList_Release,
                    },

                    D3d12Object_GetPrivateData,
                    D3d12Object_SetPrivateData,
                    D3d12Object_SetPrivateDataInterface,
                    D3d12Object_SetName,
                },

                D3d12DeviceChild_GetDevice,
            },

            D3d12CommandList_GetType,
        },

        D3D12VideoProcessCommandList_Close,
        D3D12VideoProcessCommandList_Reset,
        D3D12VideoProcessCommandList_ClearState,
        D3D12VideoProcessCommandList_ResourceBarrier,
        D3D12VideoProcessCommandList_DiscardResource,
        D3D12VideoProcessCommandList_BeginQuery,
        D3D12VideoProcessCommandList_EndQuery,
        D3D12VideoProcessCommandList_ResolveQueryData,
        D3D12VideoProcessCommandList_SetPredication,
        D3D12VideoProcessCommandList_SetMarker,
        D3D12VideoProcessCommandList_BeginEvent,
        D3D12VideoProcessCommandList_EndEvent,
        D3D12VideoProcessCommandList_ProcessFrames,
        D3D12VideoProcessCommandList_WriteBufferImmediate,

    };
#endif


    D3DINTERFACE(ID3D12VideoProcessCommandList, AEB2543A, 167F, 4682, AC, C8, D1, 59, ED, 4A, 62, 09) : public ID3D12CommandList
    {
    public:

        D3DINLINE D3D12VideoProcessCommandList_Function_Table* CommandListFunction()
        {
            return (D3D12VideoProcessCommandList_Function_Table*)m_pFunction;
        }


        D3DINLINE HRESULT D3DAPI Close()
        {
            return CommandListFunction()->Close(this);
        }

        D3DINLINE HRESULT D3DAPI Reset(
            _In_  ID3D12CommandAllocator *pAllocator)
        {
            return CommandListFunction()->Reset(this, pAllocator);
        }

        D3DINLINE void D3DAPI ClearState()
        {
            return CommandListFunction()->ClearState(this);
        }

        D3DINLINE void D3DAPI ResourceBarrier(
             _In_  UINT NumBarriers,
            _In_reads_(NumBarriers)  const D3D12_RESOURCE_BARRIER *pBarriers)
        {
            return CommandListFunction()->ResourceBarrier(this, NumBarriers, pBarriers);
        }

        D3DINLINE void D3DAPI DiscardResource(
             _In_  ID3D12Resource *pResource,
            _In_opt_  const D3D12_DISCARD_REGION *pRegion)
        {
            return CommandListFunction()->DiscardResource(this, pResource, pRegion);
        }

        D3DINLINE void D3DAPI BeginQuery(
             _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT Index)
        {
            return CommandListFunction()->BeginQuery(this, pQueryHeap, Type, Index);
        }

        D3DINLINE void D3DAPI EndQuery(
             _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT Index)
        {
            return CommandListFunction()->EndQuery(this, pQueryHeap, Type, Index);
        }

        D3DINLINE void D3DAPI ResolveQueryData(
             _In_  ID3D12QueryHeap *pQueryHeap,
            _In_  D3D12_QUERY_TYPE Type,
            _In_  UINT StartIndex,
            _In_  UINT NumQueries,
            _In_  ID3D12Resource *pDestinationBuffer,
            _In_  UINT64 AlignedDestinationBufferOffset)
        {
            return CommandListFunction()->ResolveQueryData(this, pQueryHeap, Type, StartIndex, NumQueries, pDestinationBuffer, AlignedDestinationBufferOffset);
        }

        D3DINLINE void D3DAPI SetPredication(
             _In_opt_  ID3D12Resource *pBuffer,
            _In_  UINT64 AlignedBufferOffset,
            _In_  D3D12_PREDICATION_OP Operation)
        {
            return CommandListFunction()->SetPredication(this, pBuffer, AlignedBufferOffset, Operation);
        }

        D3DINLINE void D3DAPI SetMarker(
             UINT Metadata,
            _In_reads_bytes_opt_(Size)  const void *pData,
            UINT Size)
        {
            return CommandListFunction()->SetMarker(this, Metadata, pData, Size);
        }

        D3DINLINE void D3DAPI BeginEvent(
             UINT Metadata,
            _In_reads_bytes_opt_(Size)  const void *pData,
            UINT Size)
        {
            return CommandListFunction()->BeginEvent(this, Metadata, pData, Size);
        }

        D3DINLINE void D3DAPI EndEvent()
        {
            return CommandListFunction()->EndEvent(this);
        }

        D3DINLINE void D3DAPI ProcessFrames(
        _In_  ID3D12VideoProcessor *pVideoProcessor,
        _In_  const D3D12_VIDEO_PROCESS_OUTPUT_STREAM_ARGUMENTS *pOutputArguments,
        UINT NumInputStreams,
        _In_  const D3D12_VIDEO_PROCESS_INPUT_STREAM_ARGUMENTS *pInputArguments)
        {
            return CommandListFunction()->ProcessFrames(this, pVideoProcessor, pOutputArguments, NumInputStreams, pInputArguments);
        }

        D3DINLINE void D3DAPI WriteBufferImmediate(
            UINT Count,
            _In_reads_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER *pParams,
            _In_reads_opt_(Count)  const D3D12_WRITEBUFFERIMMEDIATE_MODE *pModes)
        {
            return CommandListFunction()->WriteBufferImmediate(this, Count, pParams, pModes);
        }
    };

    // external c
};


#pragma warning( pop )

#endif // __d3d12_x_h__

