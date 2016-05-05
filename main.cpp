#include "main.h"
#include <cmath>
#include "mygl.h"
#include <vector>

#define PI 3.1415

#define VERMELHO 255,0,0,255
#define VERDE 0,255,0,255
#define AZUL 0,0,255,255
#define BRANCO 255,255,255,255

#define D 1  // Near plane
//#define F 50.0  // Far plane

objLoader *objData;
float phi = 0;

void CleanScreen(void){
	for(int i = 0; i < IMAGE_WIDTH-1; i++){
		for(int j = 0; j < IMAGE_HEIGHT-1; j++){
			int MemPos = i*4 + j*IMAGE_WIDTH*4;

			FBptr[MemPos] = 0;
			FBptr[MemPos + 1] = 0;
			FBptr[MemPos + 2] = 0;
			FBptr[MemPos + 3] = 0;
		}
	}
}

void CalcDistort(Matrix4f &Transf)
{
	Vector4f vertex[3];

	for(int i = 0; i < objData->faceCount; i++)
	{
		obj_face *o = objData->faceList[i];
		for(int j = 0; j < 3; j++){
			for(int k = 0; k < 3; k++){
				vertex[j](k) = objData->vertexList[o->vertex_index[j]]->e[k];
			}

			vertex[j](3) = 1;
			vertex[j] = Transf * vertex[j];
		}

		Triangle *faces = new Triangle(new Pixel(BRANCO, vertex[0](0), vertex[0](1)),
					   	   	   	   	   new Pixel(BRANCO, vertex[1](0), vertex[1](1)),
									   new Pixel(BRANCO, vertex[2](0), vertex[2](1)));

		faces->DrawTriangle();
		delete faces;
	}
}

//-----------------------------------------------------------------------------

void MyGlDraw(void)
{
	Matrix4f Model, View, Projection, Canonical, ModelViewPort;
	Matrix4f Model_T, Model_R; // Model translation and rotation
	Matrix4f Projection_P, Projection_T;
	Matrix4f ModelViewPort_T, ModelViewPort_S1, ModelViewPort_S2; // ModelViewPort translation and scale
	Matrix4f Cam_Bt, Cam_T;

	Vector3f CamX, CamY, CamZ;
	Vector3f Cam_pos(0, 0, 4),
			 Cam_lookat(0, 0, 0),
			 Cam_up(0, 1, 0);


	/**  Montando matriz de transformação Model  **/


	Model_R << cos(phi), 0, -sin(phi), 0,
			   0,        1,  0,        0,
			   sin(phi), 0,  cos(phi), 0,
			   0,        0,  0,        1;


	Model_T << 1, 0, 0, 0,
			   0, 1.5, 0, 0,
			   0, 0, 1, 0,
			   0, 0, 0, 1;

	Model = Model_T * Model_R;


	/**  Construindo Vetores da Câmera  **/

	CamZ = (Cam_pos - Cam_lookat);
	CamZ /= CamZ.norm();

	CamX = Cam_up.cross(CamZ); // Produto vetorial Cam_up x CamZ
	CamX /= CamX.norm();       // Nomrmalizando Vetor resultante CamX

	CamY = CamZ.cross(CamX); // Produto vetorial CamZ x CamX
	CamY /= CamY.norm();     // Nomrmalizando Vetor resultante CamY


	/**  Montando Matriz de transformação da câmera  **/

	Cam_Bt << CamX(0), CamX(1), CamX(2), 0,
			  CamY(0), CamY(1), CamY(2), 0,
			  CamZ(0), CamZ(1), CamZ(2), 0,
			  0,       0,       0,       1;

	Cam_T << 1, 0, 0, -Cam_pos(0),
			 0, 1, 0, -Cam_pos(1),
			 0, 0, 1, -Cam_pos(2),
			 0, 0, 0, 1;

	View = Cam_Bt*Cam_T;


	/**  Montando matriz de transformação Projection  **/

	Projection_P << 1, 0,  0,   0,
				    0, 1,  0,   0,
				    0, 0,  1,   0,
				    0, 0, -1/D, 1;

	Projection_T << 1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, D,
					0, 0, 0, 1;

	Projection = Projection_T * Projection_P;


	/**  Preparando Matriz de Transformação canônica  **/

	Canonical = Projection * View * Model;
	Canonical /= Canonical(3,3); // Dividindo pela coordenada Homogênia W


	/**  Preparando Matriz ModelViewPort, para exibir na tela  **/

	ModelViewPort_S1 << 1,  0, 0, 0,
						0, -1, 0, 0,
						0,  0, 1, 0,
						0,  0, 0, 1;

	ModelViewPort_T << 1, 0, 0, 1,
					   0, 1, 0, 1,
					   0, 0, 1, 0,
					   0, 0, 0, 1;

	ModelViewPort_S2 << (IMAGE_WIDTH-1)/2, 0,                  0, 0,
					    0,                 (IMAGE_HEIGHT-1)/2, 0, 0,
					    0,                 0,                  1, 0,
					    0,                 0,                  0, 1;

	ModelViewPort = ModelViewPort_S2 * ModelViewPort_T * ModelViewPort_S1 * Canonical;

	CleanScreen();
	CalcDistort(ModelViewPort); // Calcula a distorção no pipeline Gráfico

	if(phi < 2*PI)
		phi += PI/64;
	else
		phi = 0;

}

//-----------------------------------------------------------------------------

int main(int argc, char **argv)
{
	// Inicializações.
	InitOpenGL(&argc, argv);
	InitCallBacks();
	InitDataStructures();

	objData = new objLoader();			// cria o objeto que carrega o modelo
	objData->load("monkey_head2.obj");	// a carga do modelo é indicada atraves do nome do arquivo.
										// Neste caso, deve ser sempre do tipo OBJ.

	// Ajusta a função que chama as funções do mygl.h
	DrawFunc = MyGlDraw;	

	// Framebuffer scan loop.
	glutMainLoop();

	return 0;
}

