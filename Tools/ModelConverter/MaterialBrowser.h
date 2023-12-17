
#ifndef MATERIAL_BROWSER_H
#define MATERIAL_BROWSER_H


#ifdef __cplusplus
extern "C"
{
#endif



extern LRESULT CALLBACK MaterialBrowserDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

extern void	MaterialBrowserAddDefaultMaterials( void );
extern void MaterialBrowserDeleteAllMaterials( void );

extern void		MaterialBrowserOnFaceSelectionUpdated();


#ifdef __cplusplus
}
#endif



#endif