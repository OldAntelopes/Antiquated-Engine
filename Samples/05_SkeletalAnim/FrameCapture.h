#ifndef SKELETALANIM_SAMPLE_FRAME_CAPTURE_H
#define SKELETALANIM_SAMPLE_FRAME_CAPTURE_H

//-----------------------------------------------------------------------------------------------------------------------
// FrameCapture
//
//  Grabs the D3D9 back buffer - either to a single image file, or as frames appended to an .mp4 video
//  (H.264 via Windows Media Foundation, so no external tools needed)
//-----------------------------------------------------------------------------------------------------------------------

extern BOOL		FrameCaptureSaveScreenshot( const char* szFilename );

extern BOOL		FrameCaptureVideoStart( const char* szFilename, int nWidth, int nHeight, int nFPS );
extern BOOL		FrameCaptureVideoAddFrame( void );
extern void		FrameCaptureVideoEnd( void );

#endif
