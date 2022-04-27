#ifndef MEASUREWIDGET_H
#define MEASUREWIDGET_H

#include <QWidget>
#include "ui_measurewidget.h"

/*!
 * \class MeasureWidget
 * \brief A Widget encapsulating control and view
 *
 * The widget groups UI elements formerly within the
 * "measure" group box into one succint widget class.
 * Signals and slots are provided to interact with the
 * Manager class which owns the data model.
 *
 * Signal and slot connections should only be made
 * in the implementation of initializeConnections() method of
 * a parent Dialog class.  The widget is first initialized
 * with a QStandardItemModel under the ownership of a Manager.
 *
 * \sa ManagerBase, DialogBase
 *
 */

QT_FORWARD_DECLARE_CLASS(QStandardItemModel)

class MeasureWidget : public QWidget, public Ui::MeasureWidget
{
    Q_OBJECT

    Q_PROPERTY(int displayRowCount READ getDisplayRowCount WRITE setDisplayRowCount)

public:
    explicit MeasureWidget(QWidget *parent = Q_NULLPTR);
    ~MeasureWidget();

    void initialize(QStandardItemModel*);

    // set the measure and save buttons to disabled state
    //
    void disableMeasure();

    void setDisplayRowCount(const int& count)
    {
        if(1<count && m_displayRowCount != count)
            m_displayRowCount = 1;
    }
    int getDisplayRowCount() const
    {
        return m_displayRowCount;
    }

public slots:    
    void updateModelView();     // a manager nofified dataChanged
    void enableMeasure();       // a manager notified canMeasure
    void enableWriteToFile();   // a manager notified canWrite

signals:
    void measure();             // the measure button was clicked
    void writeToFile();         // the save button was clicked
    void closeApplication();    // the close button was clicked

private:
    Ui::MeasureWidget *ui { Q_NULLPTR };

    // the maximum number of rows to display by default
    // if there are more than the number of display rows, the
    // vertical scroll bar can still be used to view them
    //
    int m_displayRowCount;
};

#endif // MEASUREWIDGET_H
