#ifndef MESSAGEEDIT_H
#define MESSAGEEDIT_H

#include <QTextBrowser>
#include <QLabel>
#include <QVBoxLayout>
#include "msgbean.h"

#define MESSAGE_LABEL


/**
  * 消息气泡类
  */
class MessageView : public QWidget
{
    Q_OBJECT
public:
    explicit MessageView(QWidget *parent = nullptr);

    void setMessage(const MsgBean &msg, int recursion = 0);
    MessageView *setRepyMessage(const MsgBean &replyMsg, int recursion = 0);
    static QString simpleMessage(const MsgBean &msg);

    void setPrevContentWidth(int w);
    QSize adjustSizeByTextWidth(int w);

    void setTextColor(QColor c);

    void markDeleted();

    void markSelected();

    QSize sizeHint() const override;

private:
    void updateStyleSheet();

signals:
    void needMemberNames();
    void keepShowing();
    void restoreTimerIfNecessary();
    void replyText(const QString& text);
    void sendText(const QString& text);
    void focusMessage(const qint64 messageId);
    void heightChanged();
    void connectNewMessageView(MessageView* view);

public slots:
    void replaceGroupAt();

    void showMenu();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    MsgBean msg;
    int fixedWidth = 0;
    QColor textColor = Qt::black;
    int replyRecursion = 0; // 回复的级别

    bool singleImage = false; // 是否是单张图片/视频（不显示气泡）
    bool useFixedSize = false;
    bool singleCard = false; // 是否是单张卡片
    QString filePath;
    QPixmap filePixmap;

    QVBoxLayout* vlayout = nullptr;
    MessageView* replyWidget = nullptr; // 回复（不一定有）
#ifdef MESSAGE_LABEL
    QLabel* contentWidget = nullptr; // 内容控件
#else
    QTextBrowser* contentWidget = nullptr;
#endif

    bool deleted = false;
    bool selected = false;
};

#endif // MESSAGEEDIT_H
