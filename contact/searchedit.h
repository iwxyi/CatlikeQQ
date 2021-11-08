#ifndef SEARCHEDIT_H
#define SEARCHEDIT_H

#include <QLineEdit>

class SearchEdit : public QLineEdit
{
    Q_OBJECT
public:
    SearchEdit(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *) override;

signals:
    void signalKeyUp();
    void signalKeyDown();
    void signalKeyESC();
};

#endif // SEARCHEDIT_H
