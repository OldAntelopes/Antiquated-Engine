#ifndef INTERFACE_IMAGE_OPENGL_H
#define INTERFACE_IMAGE_OPENGL_H


#ifdef __cplusplus
extern "C"				// All interfaces use a C-linkage
{
#endif



	
extern void		InterfaceImagesInit( void );

extern void		InterfaceImagesUpdate( void );

extern void		InterfaceImagesFree( void );





#ifdef __cplusplus
}
#endif



#endif // #ifndef INTERFACE_IMAGE_OPENGL_H
