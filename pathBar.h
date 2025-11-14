#ifndef PATHBAR_H
#define PATHBAR_H
#pragma once
#include <QWidget>
#include <QToolButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QDir>
#include <QLabel>
#include <QMouseEvent>

class PathBar : public QWidget {
    Q_OBJECT

public:
    explicit PathBar(QWidget *parent = nullptr) : QWidget(parent) {
        stacked = new QStackedLayout(this);

        breadcrumbWidget = new QWidget(this);
        breadcrumbLayout = new QHBoxLayout(breadcrumbWidget);
        breadcrumbLayout->setContentsMargins(2, 0, 2, 0);
        breadcrumbLayout->setSpacing(3);
        stacked->addWidget(breadcrumbWidget);

        lineEdit = new QLineEdit(this);
        stacked->addWidget(lineEdit);

        stacked->setCurrentWidget(breadcrumbWidget);

        connect(lineEdit, &QLineEdit::returnPressed, this, [this]() {
            QString path = lineEdit->text().trimmed();
            emit pathEntered(path);
            setPath(path);
            stacked->setCurrentWidget(breadcrumbWidget);
        });
    }

    void setPath(const QString &path) {
        currentPath = QDir::cleanPath(path);
        clearBreadcrumb();

        QStringList parts = currentPath.split('/', Qt::SkipEmptyParts);
        QString accum = "/";

        for (int i = 0; i < parts.size(); ++i) {
            QString part = parts[i];
            QString fullPath = accum + part;

            auto *btn = new QToolButton(this);
            btn->setText(part.isEmpty() ? "/" : part);
            btn->setAutoRaise(true);
            btn->setCursor(Qt::PointingHandCursor);

            connect(btn, &QToolButton::clicked, this, [this, fullPath]() {
                emit pathClicked(fullPath);
            });

            breadcrumbLayout->addWidget(btn);

            if (i < parts.size() - 1) {
                auto *sep = new QLabel("/", this);
                breadcrumbLayout->addWidget(sep);
            }

            accum = fullPath + "/";
        }

        breadcrumbLayout->addStretch();
        lineEdit->setText(currentPath);
    }

signals:
    void pathClicked(const QString &path);
    void pathEntered(const QString &path);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            stacked->setCurrentWidget(lineEdit);
            lineEdit->setFocus();
            lineEdit->selectAll();
        }
    }


private:
    QStackedLayout *stacked;
    QWidget *breadcrumbWidget;
    QHBoxLayout *breadcrumbLayout;
    QLineEdit *lineEdit;
    QString currentPath;

    void clearBreadcrumb() {
        QLayoutItem *item;
        while ((item = breadcrumbLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
    }
};

#endif // PATHBAR_H
