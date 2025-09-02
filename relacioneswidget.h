#ifndef RELACIONESWIDGET_H
#define RELACIONESWIDGET_H
#include<QWidget>
#include<QPoint>
#include<QSet>

class QFrame;
class QLabel;


class RelacionesWidget:public QWidget
{

    Q_OBJECT

public:

   explicit RelacionesWidget(QWidget*parent=nullptr);

public slots:

    //esta funcion muestra la minitabla
    void MostrarSelectorTablas(const QStringList&tablas,bool soloSiPrimeraVez=false);

    //Agrega una mini-tabla al lienzo (duplicados permitidos; auto-sufijo _1, _2, …)
    void agregarMiniTabla(const QString& nombreBase);

    //Genera un titulo unico (Alumnos, Alumnos_1, …) segun lo que ya exista en el canvas.
    QString tituloUnico(const QString& base) const;

    // Posicionamiento simple en grilla (auto-layout)
    QPoint proximaPosicion();

private:

    QWidget*m_canvas=nullptr;//lienzo de relaciones
    mutable QSet<QString>m_titulos;//titulos ya usados
    QPoint m_siguiente=QPoint(32,32);
    int m_dx=260;//separacion horizontal
    int m_dy=200;//separacion vertical

    bool m_selectorMostrado=false;

};

#endif // RELACIONESWIDGET_H
