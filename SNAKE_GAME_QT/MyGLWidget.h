#include "LL2GLWidget.h"

#include <vector>

#include <QTimer>

class MyGLWidget : public LL2GLWidget {
  Q_OBJECT

  public:
    MyGLWidget(QWidget *parent);
    ~MyGLWidget();

  public slots:
    void changeFov(int a);
    void changeView();
    void changeEuler(int a);

  protected:
  
    virtual void keyPressEvent (QKeyEvent *event);

    virtual void mouseMoveEvent(QMouseEvent *e);

    virtual void resizeGL(int width, int height);
    // funció per comprovar si una nova posició de la serp és vàlida
    virtual bool checkPosition(glm::vec3 pos);
    virtual void paintGL();

  void iniEscena();
  void iniCamera();
  void initializeGL();
  void TerraTransform();
  void PipeTransform(glm::vec3 pos);
  void SnakeHeadTransform();
  void SnakeBodyTransform(glm::vec3 pos);
  void SnakeTailTransform();
  void MarbleTransform();
  void viewTransform();
  void projectTransform();

  void updateSnakeGame();
  void PintaPipes();
  void PintaFilaPipes(glm::vec3 posIni, glm::vec3 direccion);
  

  private:
    float escalaPipe, distancia;
    float l, r, b, t;
    float FOVini;
    float giroTheta, giroPsi, x_ant, y_ant;
    int printOglError(const char file[], int line, const char func[]);
    glm::vec3 escenaMinima = glm::vec3(-15, 0, -15);
    glm::vec3 escenaMaxima = glm::vec3(15, 3, 15);
    bool perspective;

   
};
