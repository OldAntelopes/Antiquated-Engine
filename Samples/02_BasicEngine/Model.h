#ifndef BASICENGINE_SAMPLE_MODEL_H
#define BASICENGINE_SAMPLE_MODEL_H



class Model
{
public:
	Model();
	~Model();

	int			Load( const char* szFilename );

	void		Render( void );
	
private:

	int			m_ModelHandle;

};



#endif // #ifndef BASICENGINE_SAMPLE_MODEL_H
