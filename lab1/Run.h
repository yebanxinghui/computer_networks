#include <QObject>

class Run : public QObject
{
    Q_OBJECT
public:
    explicit Run(QObject *parent = 0);
    ~Run();

signals:

public slots:
};
