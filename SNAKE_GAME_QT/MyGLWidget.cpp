#include "MyGLWidget.h"
#include <iostream>
#include <stdio.h>
#define printOpenGLError() printOglError(__FILE__, __LINE__)
#define CHECK() printOglError(__FILE__, __LINE__,__FUNCTION__)
#define DEBUG() std::cout << __FILE__ << " " << __LINE__ << " " << __FUNCTION__ << std::endl;

MyGLWidget::MyGLWidget(QWidget *parent=0) : LL2GLWidget(parent) 
{
}

int MyGLWidget::printOglError(const char file[], int line, const char func[]) 
{
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    const char * error = 0;
    switch (glErr)
    {
        case 0x0500:
            error = "GL_INVALID_ENUM";
            break;
        case 0x501:
            error = "GL_INVALID_VALUE";
            break;
        case 0x502: 
            error = "GL_INVALID_OPERATION";
            break;
        case 0x503:
            error = "GL_STACK_OVERFLOW";
            break;
        case 0x504:
            error = "GL_STACK_UNDERFLOW";
            break;
        case 0x505:
            error = "GL_OUT_OF_MEMORY";
            break;
        default:
            error = "unknown error!";
    }
    if (glErr != GL_NO_ERROR)
    {
        printf("glError in file %s @ line %d: %s function: %s\n",
                             file, line, error, func);
        retCode = 1;
    }
    return retCode;
}

MyGLWidget::~MyGLWidget()
{
}

bool MyGLWidget::checkPosition(glm::vec3 pos)
{
	bool valid = true;
        // Comprobar que no se salga del recinto
        if(pos.x >= 14 or pos.x <= -14 or pos.z >= 14 or pos.z <= -14) valid =  false;
        else {
            //Comprobar que no se coma a sí misma
            if(pos == tailPos) valid = false;
            else {
                for(auto bodyPo : bodyPos){
                    if(pos == bodyPo){
                        valid = false;
                        break;
                    }
                }
            }
        }
	return valid; 
}

void MyGLWidget::paintGL ()
{
  // descomentar per canviar paràmetres
  // glViewport (0, 0, ample, alt);

  // Esborrem el frame-buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // SnakeHead
  glBindVertexArray (VAO_models[SNAKE_HEAD]);
  SnakeHeadTransform();
  glDrawArrays(GL_TRIANGLES, 0, models[SNAKE_HEAD].faces().size()*3);
  
  // SnakeBody
  glBindVertexArray (VAO_models[SNAKE_BODY]);
  for (unsigned int i = 0; i < bodyPos.size(); i++)
  {  
	  SnakeBodyTransform(bodyPos[i]);
	  glDrawArrays(GL_TRIANGLES, 0, models[SNAKE_BODY].faces().size()*3);
  }

  // SnakeTail
  glBindVertexArray (VAO_models[SNAKE_TAIL]);
  SnakeTailTransform();
  glDrawArrays(GL_TRIANGLES, 0, models[SNAKE_TAIL].faces().size()*3);
  
  // Marble
  glBindVertexArray (VAO_models[MARBLE]);
  MarbleTransform();
  glDrawArrays(GL_TRIANGLES, 0, models[MARBLE].faces().size()*3);

  // Pipe
  PintaPipes();

  // Terra
  glBindVertexArray (VAO_Terra);
  TerraTransform();
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glBindVertexArray (0);
}

void MyGLWidget::initializeGL ()
{
  // Cal inicialitzar l'ús de les funcions d'OpenGL
  initializeOpenGLFunctions();  
  glEnable(GL_DEPTH_TEST);
  
  glClearColor(0.5, 0.7, 1.0, 1.0); // defineix color de fons (d'esborrat)
  carregaShaders();
  creaBuffersModels();
  creaBuffersTerra();
  iniEscena();

  iniCamera();
}

void MyGLWidget::iniEscena(){
  radiEscena = distance(escenaMinima, escenaMaxima)/2.0;
  centreEscena = (escenaMaxima + escenaMinima);

  headPos = glm::vec3(0,0,0);
  headAngle = -90;
  bodyPos.clear();	    
  bodyPos.push_back(glm::vec3(-1,0,0));    	
  tailPos = glm::vec3(-2,0,0);
  tailAngle = 0;

  marblePos = glm::vec3(10, 0, 0);

  direction = glm::vec3(1,0,0);

  perspective = true;
}

void MyGLWidget::iniCamera(){
    distancia = radiEscena * 2;
    znear =  distancia - radiEscena;
    zfar  = distancia + radiEscena;
    giroTheta = 45;
    giroPsi = 0.0;

    l = b = -radiEscena;
    r = t = radiEscena;
    obs = centreEscena + distancia*glm::vec3(0, 1, 0);
    vrp = centreEscena;
    up = glm::vec3(0, 0, -1);
    FOVini = 1.295;
    fov = FOVini;
    ra = 1.0;


    projectTransform();
    viewTransform();
}

void MyGLWidget::keyPressEvent(QKeyEvent* event)
{
    makeCurrent();
    switch (event->key()) {
        case Qt::Key_Up: {
            direction = glm::vec3(0, 0, -1);
            break;
        }
        case Qt::Key_Down: {
            direction = glm::vec3(0, 0, 1);
            break;
        }
        case Qt::Key_Left: {
            direction = glm::vec3(-1, 0, 0);
            break;
        }
        case Qt::Key_Right: {
            direction = glm::vec3(1, 0, 0);
            break;
        }
        case Qt::Key_C: {
            perspective = !perspective;
            viewTransform();
            projectTransform();
            break;
        }
        case Qt::Key_R: {
            initializeGL();
            break;
        }
        default:
            event->ignore();
            break;
    }
    updateSnakeGame();
    update();


}

void MyGLWidget::updateSnakeGame()
{
    glm::vec3 newPos = headPos + direction;

    if (checkPosition(newPos)) // check boundary limits, check if eats himself
    {
        bodyPos.push_back(headPos);
        headPos = newPos;

        if (direction == glm::vec3(0,0,-1))
            headAngle = -180;
        if (direction == glm::vec3(0,0,1))
            headAngle = 180;
        if (direction == glm::vec3(-1,0,0))
            headAngle = 90;
        if (direction == glm::vec3(1,0,0))
            headAngle = -90;

        if (glm::distance(newPos,marblePos) > 0) // check if doesn't eat marble
        {
            tailPos = bodyPos.front();
            bodyPos.erase(bodyPos.begin());
            glm::vec3 tailMov = bodyPos.front() - tailPos;
            tailAngle = signedAngleBetweenVectors(glm::vec3(1,0,0),tailMov);
        }
        else
            computeRandomMarblePosition();
    }
}

void MyGLWidget::viewTransform(){
    glm::mat4 View(1.0);
    if(perspective) {
        View = glm::translate(View, glm::vec3(0., 0., -distancia));
        View = glm::rotate(View, glm::radians(giroTheta), glm::vec3(1., 0., 0.));
        View = glm::rotate(View, glm::radians(-giroPsi), glm::vec3(0., 1., 0.));
        View = glm::translate(View, -centreEscena);
    }
    else {
        View = glm::lookAt(obs, vrp, up);
    }

    glUniformMatrix4fv (viewLoc, 1, GL_FALSE, &View[0][0]);
}

void MyGLWidget::projectTransform(){
    glm::mat4 Proj(1.0f);
    if(perspective) Proj = glm::perspective(fov, ra, znear, zfar);
    else Proj = glm::ortho(l, r, b, t, znear, zfar);

    glUniformMatrix4fv (projLoc, 1, GL_FALSE, &Proj[0][0]);
}

void MyGLWidget::resizeGL(int w, int h){
    float rav = float(w) / float(h);
    ra = rav;
    if(rav < 1.0) fov = 2.0*atan(tan(FOVini/2.0)/rav);
    projectTransform();
    glViewport(0, 0, w, h);
}


void MyGLWidget::PintaPipes(){

    std::vector<glm::vec3> posiciones = {glm::vec3(14.5, 0, 14.5), glm::vec3(-14.5, 0, 14.5), glm::vec3(-14.5, 0, -14.5), glm::vec3(14.5, 0, -14.5)};
    std::vector<glm::vec3> direcciones = {glm::vec3(1,0,0), glm::vec3(0, 0, 1), glm::vec3(-1, 0, 0), glm::vec3(0, 0, -1)};
    for(unsigned long i = 0; i < posiciones.size(); ++i){
        PintaFilaPipes(posiciones[i], direcciones[i]);
    }

}

void MyGLWidget::PintaFilaPipes(glm::vec3 posIni, glm::vec3 direccion){
  glm::vec3 posAct;

  for(float i = 0; i < 30; ++i){
    posAct = posIni - (i*direccion);
    glBindVertexArray (VAO_models[PIPE]);
    PipeTransform(posAct);
    glDrawArrays(GL_TRIANGLES, 0, models[PIPE].faces().size()*3);
  }
}


void MyGLWidget::TerraTransform(){
  
  glm::mat4 TG(1.0f);
  TG = glm::translate(TG, glm::vec3(0,0,0));
  TG = glm::scale(TG, glm::vec3(3,3,3));
  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}


void MyGLWidget::PipeTransform (glm::vec3 pos)
{
  escalaPipe = 3*escalaModels[PIPE];
  glm::mat4 TG(1.0f);
  TG = glm::translate(TG, pos);
  TG = glm::scale(TG, glm::vec3(escalaModels[PIPE], escalaPipe, escalaModels[PIPE]));                    //le ponemos h = 3
  TG = glm::translate(TG, -centreBaseModels[PIPE]);
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::SnakeHeadTransform ()
{
  glm::mat4 TG(1.0f);  
  TG = glm::translate(TG, headPos);
  TG = glm::scale(TG, glm::vec3(escalaModels[SNAKE_HEAD]));
  TG = glm::rotate(TG, glm::radians(headAngle), glm::vec3(0,1,0));
  TG = glm::translate(TG, -centreBaseModels[SNAKE_HEAD]);
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::SnakeBodyTransform (glm::vec3 pos)
{
  glm::mat4 TG(1.0f);
  TG = glm::translate(TG, pos);
  TG = glm::scale(TG, glm::vec3(escalaModels[SNAKE_BODY]));
  TG = glm::translate(TG, -centreBaseModels[SNAKE_BODY]);
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::SnakeTailTransform ()
{
  glm::mat4 TG(1.0f);
  TG = glm::translate(TG, tailPos);
  TG = glm::scale(TG, glm::vec3(escalaModels[SNAKE_TAIL]));
  TG = glm::rotate(TG, glm::radians(tailAngle), glm::vec3(0,1,0));
  TG = glm::translate(TG, -centreBaseModels[SNAKE_TAIL]);
  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::MarbleTransform ()
{
  glm::mat4 TG(1.0f);
  TG = glm::translate(TG, marblePos);
  TG = glm::scale(TG, glm::vec3(0.5*escalaModels[MARBLE]));
  TG = glm::translate(TG, -centreBaseModels[MARBLE]);
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}



void MyGLWidget::mouseMoveEvent(QMouseEvent *e) {
	makeCurrent();
	
	if(e->x() > x_ant){
        giroPsi -= 1.5;
        std::cout << giroPsi << std::endl;
    }
	else if(e->x() < x_ant) giroPsi += 1.5;
	
	if(e->y() > y_ant) giroTheta += 1.5;
	else if(e->y() < y_ant) giroTheta -= 1.5;
	
	x_ant = e->x();
	y_ant = e->y();
	viewTransform();
	update();
}

//slots 

void MyGLWidget::changeFov(int a) {
    makeCurrent();
    fov = 0.5 + a*0.159;
    projectTransform();
    update();
}

void MyGLWidget::changeView() {
    makeCurrent();
    perspective = !perspective;
    viewTransform();
    update();
}

void MyGLWidget::changeEuler(int a) {

}


