#pragma once

#include <QWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QPainter>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPixmap>
#include <QLabel>

class MapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);
    ~MapWidget();

    void setMapLayer(const QString &layerType);
    void resetView();
    void zoomIn();
    void zoomOut();
    void setCenter(double latitude, double longitude);
    void setZoom(int zoom);

    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    int zoom() const { return m_zoom; }

signals:
    void coordinateChanged(double latitude, double longitude);
    void zoomChanged(int zoom);
    void mapClicked(double latitude, double longitude);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void updateCoordinates();
    void onTileDownloaded();

private:
    void setupMap();
    void updateMapDisplay();
    void loadMapTile(int x, int y, int zoom);
    QPointF screenToMapCoordinate(const QPointF &screenPoint) const;
    void paintEvent(QPaintEvent *event) override;
    
    // Map state
    double m_latitude;
    double m_longitude;
    int m_zoom;
    QString m_currentLayer;
    
    // Interaction state
    bool m_dragging;
    QPointF m_lastPanPoint;
    QTimer *m_updateTimer;
    
    // Network manager for tile loading
    QNetworkAccessManager *m_networkManager;
    
    // Map display
    QPixmap m_mapPixmap;
    
    // Default map settings
    static constexpr double DEFAULT_LATITUDE = 39.8283;  // Washington, DC
    static constexpr double DEFAULT_LONGITUDE = -98.5795; // Center of USA
    static constexpr int DEFAULT_ZOOM = 4;
    static constexpr int MIN_ZOOM = 1;
    static constexpr int MAX_ZOOM = 18;
};