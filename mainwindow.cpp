#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){

    ui->setupUi(this);
    dirRaiz = QDir::homePath() + "/" + "Dermasoft - Historias";
    fecha = QDate::currentDate().toString("dd.MM.yyyy");
    tmrTimer = new QTimer(this);
    //conecta el timer con la actualizacion del cuadro de la GUI
    connect(tmrTimer, SIGNAL(timeout()), this, SLOT(procesarCuadroActualizarGUI()));
    QDir dir;
    dir.mkpath(dirRaiz);
    ui->etqInfo->setAlignment(Qt::AlignCenter);
    //deshabilitar todos los botones necesarios al iniciar la interfaz
    ui->actionOpciones->setEnabled(false);
    ui->actionDesconectar_camara->setEnabled(false);
    ui->actionCerrar_historia->setEnabled(false);
    ui->actionEliminar_historia->setEnabled(false);
    setBotones(false);
    ui->cBoxModo->setEnabled(false);
    ui->actionCrear_historia->setEnabled(false);
    ui->btnGenerarReporte->setEnabled(false);
    msjConectar = "<p align='center'><span style=' font-weight:600;'>Conectar la camara para continuar</span></p>";
    msjHistoria = "<p align='center'><span style=' font-weight:600;'>Crear o abrir una historia para continuar</span></p>";
    ui->etqInfo->setText(msjConectar);
    //si la camara esta conectada, actualiza la etiqueta de informacion.
    if(capWebcam.isOpened())
        ui->etqInfo->setText(msjHistoria);

    this->adjustSize();
    this->setFixedSize(this->size());
}

MainWindow::~MainWindow(){

    delete ui;
}

void MainWindow::procesarCuadroActualizarGUI(){

    bool flag = capWebcam.read(matOriginal);

    if(!flag){
        tmrTimer->stop();
        return;
    }
    //se prepara la imagen para convertirla de BGR A RGB para que Qt pueda manejarla
    cv::cvtColor(matOriginal, matOriginal, CV_BGR2RGB);
    //ocurre la conversion de cv mat a qimage. Investigar sobre cual QImage::Format_ es mas apropiado
    QImage imgaux((uchar*)matOriginal.data, matOriginal.cols, matOriginal.rows, matOriginal.step, QImage::Format_RGB888);
    qimg = imgaux;
    //se obtiene al largo y ancho de la imagen capturada
    int w = ui->etqCamara->width();
    int h = ui->etqCamara->height();
    //se le asigna la imagen a la etiqueta de la camara
    ui->etqCamara->setPixmap((QPixmap::fromImage(qimg)).scaled(w, h, Qt::KeepAspectRatio));
}

void MainWindow::on_actionAcerca_de_triggered(){

    AcercaDe mensaje;
    mensaje.setModal(true);
    mensaje.exec();
}

void MainWindow::on_actionSalir_triggered(){ close();}

void MainWindow::on_actionConectar_camara_triggered(){

    //si la camara no ha sido abierta
    if(capWebcam.isOpened() == false){

        capWebcam.open(0);/*Abre la camara web*/

        if(capWebcam.isOpened() == false){
            dlgInfo info("Porfavor verifique que la camara este conectada.", "Error al conectar la camara");
            info.exec();
        }else{
            ui->actionConectar_camara->setEnabled(false);
            ui->actionDesconectar_camara->setEnabled(true);
            revisionHistoria();
            dlgInfo info("La camara fue conectada correctamente.", "Camara conectada");
            info.exec();
        }
    }
}

void MainWindow::on_actionDesconectar_camara_triggered(){

    capWebcam.release();
    //borra la imagen que haya quedado en la etiqueta de la camara
    QPixmap pixDummy(0, 0);
    ui->etqCamara->setPixmap(pixDummy);
    ui->etqVistaprevia->setPixmap(pixDummy);
    ui->actionConectar_camara->setEnabled(true);
    ui->actionDesconectar_camara->setEnabled(false);
    ui->cBoxModo->setCurrentIndex(0);
    ui->etqInfo->setText(msjConectar);
    revisionHistoria();
    dlgInfo info("La camara fue desconectada correctamente.", "Camara desconectada");
    info.exec();
}

void MainWindow::on_actionOpciones_triggered(){

    Opciones opc;
    opc.setModal(true);
    opc.exec();
}

void MainWindow::on_actionCrear_historia_triggered(){

    QDir dir;
    crearHistoria crear;
    //obtiene el ID de la historia a crear
    crear.setModal(true);
    crear.exec();

    //crea la estructura de directorios necesaria para la nueva historia
    historia = crear.getHistoriaCreada();
    lesion = crear.getLesionCreada();
    dir.mkpath(dirRaiz + "/" + historia + "/" + lesion);

    //revisa que la historia este cargada para activar los botones pertinentes
    revisionHistoria();
}

void MainWindow::on_actionCerrar_historia_triggered(){

    historia = "";
    revisionHistoria();
}

void MainWindow::on_actionCerrar_lesion_triggered(){

    lesion = "";
    revisionHistoria();
}

//Si hay una historia cargada, habilita los botones pertinentes, caso contrario los deshabilita
void MainWindow::revisionHistoria(){

    //si hay una historia cargada, habilita la seleccion de colores
    if(!historia.isEmpty()){
        QString hist("Historia: " + historia + ", lesion: " + lesion);
        ui->etqInfo->setText("<p align='center'><span style=' font-weight:600;'>" + hist + ", fecha: " + fecha + "</span></p>");
        //deshabilita la opcion de crear o de abrir una historia, mientras ya este una cargada
        ui->actionCrear_historia->setEnabled(false);
        ui->actionAbrir_historia->setEnabled(false);
        //habilita la opcion de cerrar la historia cargada
        ui->actionCerrar_historia->setEnabled(true);
        ui->cBoxModo->setEnabled(true);
        ui->btnGenerarReporte->setEnabled(true);
    }else{
        ui->cBoxModo->setEnabled(false);

        //si la camara sigue activa, muestra el mensaje siguiente
        if(capWebcam.isOpened()){
            ui->actionCrear_historia->setEnabled(true);
            ui->etqInfo->setText(msjHistoria);
        }else{
            ui->actionCrear_historia->setEnabled(false);
            ui->etqInfo->setText(msjConectar);
        }
        //habilita la opcion de crear o de abrir una historia, en caso de que no haya una historia cargada
        ui->actionAbrir_historia->setEnabled(true);
        //deshabilita la opcion de cerrar la historia cargada
        ui->actionCerrar_historia->setEnabled(false);
        ui->btnGenerarReporte->setEnabled(false);
    }
    ui->cBoxModo->setCurrentIndex(0);
}

/*Indice 0: Seleccionar
  Indice 1: Capturar
  Indice 2: Visualizar*/
void MainWindow::on_cBoxModo_currentIndexChanged(int index){

    //si se elige modo de captura o modo de visualizacion
    if(index != 0){
        //si la seleccion esta en modo captura y la camara esta activa, habilita los botones para capturar las fotos
        if(index == 1){
            if(!tmrTimer->isActive())
                tmrTimer->start(30);

            if(capWebcam.isOpened())
                setBotones(true);
            else
                setBotones(false);
        }

        if(index == 2){
            if(tmrTimer->isActive())
                tmrTimer->stop();
            QPixmap pixDummy(0, 0);
            ui->etqCamara->setPixmap(pixDummy);
            //si la camara esta en modo de visualizacion
            QStringList colores;

            colores << "Amarillo" << "Azul" << "Blanco" << "Cyan" << "Magenta" << "Rojo" << "Verde";

            for(int i = 0; i < colores.size(); i++){

                setColorDisponible(colores.indexOf(colores.at(i)));
            }
        }

    }else{
        if(tmrTimer->isActive())
            tmrTimer->stop();
        //borra la imagen que este en la vista previa y en la camara
        QPixmap pixDummy(0, 0);
        ui->etqCamara->setPixmap(pixDummy);
        ui->etqVistaprevia->setPixmap(pixDummy);
        //desabilitar todos los botones, excepto seleccionar
        setBotones(false);
    }
}

void MainWindow::setBotones(bool flag){

    ui->btnAmarillo->setEnabled(flag);
    ui->btnAzul->setEnabled(flag);
    ui->btnBlanco->setEnabled(flag);
    ui->btnCyan->setEnabled(flag);
    ui->btnMagenta->setEnabled(flag);
    ui->btnRojo->setEnabled(flag);
    ui->btnVerde->setEnabled(flag);
}

void MainWindow::accionBotones(QString color){

    QString nombreImagen(dirRaiz + "/" + historia + "/" + fecha + "/" + color + " - " + fecha + ".jpg");
    QFileInfo archivo(nombreImagen);
    int w, h;

    //si la interfaz esta en modo de captura de imagenes
    if(ui->cBoxModo->currentIndex() == 1){

        QImage imgCapturada(qimg.copy());

        QDir dir;
        dir.mkpath(dirRaiz + "/" + historia + "/" + fecha);

        QString nombreImagen(dirRaiz + "/" + historia + "/" + fecha + "/" + color + " - " + fecha + ".jpg");
        QFileInfo archivo(nombreImagen);

        if(!archivo.exists()){
            imgCapturada.save(dirRaiz + "/" + historia + "/" + fecha + "/" + color + " - " + fecha + ".jpg");
            w = ui->etqVistaprevia->width();
            h = ui->etqVistaprevia->height();
            //se le asigna la imagen a la etiqueta de la camara
            ui->etqVistaprevia->setPixmap((QPixmap::fromImage(qimg)).scaled(w, h, Qt::KeepAspectRatio));
        }else{

            dlgReemplazar *dlg = new dlgReemplazar(color);
            dlg->setModal(true);
            dlg->exec();

            bool reemplazar = dlg->getReemplazar();

            if(reemplazar){
                imgCapturada.save(dirRaiz + "/" + historia + "/" + fecha + "/" + color + " - " + fecha + ".jpg");
                w = ui->etqVistaprevia->width();
                h = ui->etqVistaprevia->height();
                //se le asigna la imagen a la etiqueta de la camara
                ui->etqVistaprevia->setPixmap((QPixmap::fromImage(qimg)).scaled(w, h, Qt::KeepAspectRatio));
            }
        }

    }else{
    //si la interfaz esta en modo de visualizacion de imagenes
        dlgImagen *imagen = new dlgImagen(nombreImagen, color, dirRaiz + "/" + historia + "/" + fecha);
        imagen->setModal(true);
        imagen->exec();
    }

    if(archivo.exists()){

        QImage imagen(nombreImagen);
        w = ui->etqVistaprevia->width();
        h = ui->etqVistaprevia->height();
        ui->etqVistaprevia->setPixmap((QPixmap::fromImage(imagen)).scaled(w, h, Qt::KeepAspectRatio));
    }else{
        //borra la imagen que este en la vista previa
        QPixmap pixDummy(0, 0);
        ui->etqVistaprevia->setPixmap(pixDummy);
    }
}

void MainWindow::setColorDisponible(int colorIndex){

    QString nombreImagen;
    QFileInfo archivo;
    switch (colorIndex) {

    case 0:
        nombreImagen = (dirRaiz + "/" + historia + "/" + fecha + "/" + "Amarillo - " + fecha + ".jpg");
        archivo.setFile(nombreImagen);
        if(archivo.exists())
            ui->btnAmarillo->setEnabled(true);
        else
            ui->btnAmarillo->setEnabled(false);
        break;

    case 1:
        nombreImagen = (dirRaiz + "/" + historia + "/" + fecha + "/" + "Azul - " + fecha + ".jpg");
        archivo.setFile(nombreImagen);
        if(archivo.exists())
            ui->btnAzul->setEnabled(true);
        else
            ui->btnAzul->setEnabled(false);
        break;

    case 2:
        nombreImagen = (dirRaiz + "/" + historia + "/" + fecha + "/" + "Blanco - " + fecha + ".jpg");
        archivo.setFile(nombreImagen);
        if(archivo.exists())
            ui->btnBlanco->setEnabled(true);
        else
            ui->btnBlanco->setEnabled(false);
        break;

    case 3:
        nombreImagen = (dirRaiz + "/" + historia + "/" + fecha + "/" + "Cyan - " + fecha + ".jpg");
        archivo.setFile(nombreImagen);
        if(archivo.exists())
            ui->btnCyan->setEnabled(true);
        else
            ui->btnCyan->setEnabled(false);
        break;

    case 4:
        nombreImagen = (dirRaiz + "/" + historia + "/" + fecha + "/" + "Magenta - " + fecha + ".jpg");
        archivo.setFile(nombreImagen);
        if(archivo.exists())
            ui->btnMagenta->setEnabled(true);
        else
            ui->btnMagenta->setEnabled(false);
        break;

    case 5:
        nombreImagen = (dirRaiz + "/" + historia + "/" + fecha + "/" + "Rojo - " + fecha + ".jpg");
        archivo.setFile(nombreImagen);
        if(archivo.exists())
            ui->btnRojo->setEnabled(true);
        else
            ui->btnRojo->setEnabled(false);
        break;

    case 6:
        nombreImagen = (dirRaiz + "/" + historia + "/" + fecha + "/" + "Verde - " + fecha + ".jpg");
        archivo.setFile(nombreImagen);
        if(archivo.exists())
            ui->btnVerde->setEnabled(true);
        else
            ui->btnVerde->setEnabled(false);
        break;

    default:
        break;
    }
}

void MainWindow::on_btnRojo_clicked()
{
    accionBotones("Rojo");
}

void MainWindow::on_btnVerde_clicked()
{
    accionBotones("Verde");
}

void MainWindow::on_btnAzul_clicked()
{
    accionBotones("Azul");
}

void MainWindow::on_btnCyan_clicked()
{
    accionBotones("Cyan");
}

void MainWindow::on_btnMagenta_clicked()
{
    accionBotones("Magenta");
}

void MainWindow::on_btnAmarillo_clicked()
{
    accionBotones("Amarillo");
}

void MainWindow::on_btnBlanco_clicked()
{
    accionBotones("Blanco");
}

void MainWindow::on_btnGenerarReporte_clicked()
{
    QTextDocument doc;
}
