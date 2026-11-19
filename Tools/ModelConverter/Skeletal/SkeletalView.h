#ifndef MODELCONV_SKELETAL_VIEW_H
#define MODELCONV_SKELETAL_VIEW_H

//-----------------------------------------------------------------------------------------------------------
// SkeletalView
//
//  How the converter shows and protects skinned models (see Pub\skeletalanimation.md):
//   - The toolbar's animation combo / play / pause work on a skinned model's linked animations
//   - Skeleton overlay (bones and joints) and a bind pose toggle
//   - Reports the skeleton and its links when a model is loaded (and says which .bvh files are missing)
//   - Keeps edits from fighting the skinning: commands run on the bind pose, which is then taken from the mesh again
//   - Save As copies the linked .bvh files next to the new .atm
//-----------------------------------------------------------------------------------------------------------

typedef void (*fnSkeletalViewProject)( VECT* pxWorldPos, int* pnScreenX, int* pnScreenY );

extern void		SkeletalViewSetShowBones( BOOL bFlag );
extern BOOL		SkeletalViewGetShowBones( void );
extern void		SkeletalViewSetShowBindPose( BOOL bFlag );
extern BOOL		SkeletalViewGetShowBindPose( void );

// Each frame. Returns 1 if the model's skeletal animations now own the toolbar's animation controls, 0 if not,
// or -1 if not and the animation combo needs its usual list back
extern int		SkeletalViewUpdateAnimControls( int hModel, BOOL bVertexEditMode );
// The animation combo's selection changed. Returns TRUE if it was handled as a skeletal animation
extern BOOL		SkeletalViewOnAnimComboSelect( int hModel, int nSelection );

// 2D overlay, after the model is drawn
extern void		SkeletalViewDrawOverlay( int hModel, fnSkeletalViewProject fnProject );

extern void		SkeletalViewReportModel( int hModel );

// Around every menu / button command
extern void		SkeletalViewBeginCommand( int hModel );
extern void		SkeletalViewEndCommand( int hModel );

// After the model is saved as szNewModelFilename
extern void		SkeletalViewCopyLinkedAnimations( int hModel, const char* szNewModelFilename );

// Copies an animation file into szModelFilename's folder (if it isn't already there), asking before replacing a different
// file of the same name. szPathOut gets the file to use. Returns FALSE if the user cancelled or the copy failed
extern BOOL		SkeletalViewCopyAnimationToModelFolder( const char* szAnimFile, const char* szModelFilename, char* szPathOut, int nPathOutLen );

#endif
