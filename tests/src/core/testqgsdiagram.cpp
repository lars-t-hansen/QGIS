/***************************************************************************
     testqgsdiagram.cpp
     --------------------------------------
    Date                 : Sep 7 2012
    Copyright            : (C) 2012 by Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QPainter>

//qgis includes...
// #include <qgisapp.h>
#include <diagram/qgspiediagram.h>
#include <qgsdiagramrenderer.h>
#include <qgsmaplayer.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsrenderer.h>
//qgis test includes
#include "qgsmultirenderchecker.h"
#include "qgspallabeling.h"
#include "qgsproject.h"

/** \ingroup UnitTests
 * Unit tests for the diagram renderer
 */
class TestQgsDiagram : public QObject
{
    Q_OBJECT

  public:
    TestQgsDiagram()
        : mTestHasError( false )
        , mMapSettings( 0 )
        , mPointsLayer( 0 )
    {}

  private:
    bool mTestHasError;
    QgsMapSettings *mMapSettings;
    QgsVectorLayer *mPointsLayer;
    QString mTestDataDir;
    QString mReport;

    bool imageCheck( const QString& theTestType );

  private slots:
    // will be called before the first testfunction is executed.
    void initTestCase()
    {
      mTestHasError = false;
      QgsApplication::init();
      QgsApplication::initQgis();
      QgsApplication::showSettings();

      mMapSettings = new QgsMapSettings();

      //create some objects that will be used in all tests...

      //
      //create a non spatial layer that will be used in all tests...
      //
      QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
      mTestDataDir = myDataDir + '/';

      //
      //create a point layer that will be used in all tests...
      //
      QString myPointsFileName = mTestDataDir + "points.shp";
      QFileInfo myPointFileInfo( myPointsFileName );
      mPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                         myPointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

      // Create map composition to draw on
      mMapSettings->setLayers( QList<QgsMapLayer*>() << mPointsLayer );

      mReport += QLatin1String( "<h1>Diagram Tests</h1>\n" );
    }

    // will be called after the last testfunction was executed.
    void cleanupTestCase()
    {
      delete mMapSettings;
      delete mPointsLayer;

      QString myReportFile = QDir::tempPath() + "/qgistest.html";
      QFile myFile( myReportFile );
      if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
      {
        QTextStream myQTextStream( &myFile );
        myQTextStream << mReport;
        myFile.close();
        //QDesktopServices::openUrl( "file:///" + myReportFile );
      }
      QgsApplication::exitQgis();
    }

    // will be called before each testfunction is executed
    void init()
    {
      mPointsLayer->setDiagramRenderer( 0 );
      QgsDiagramLayerSettings dls;
      mPointsLayer->setDiagramLayerSettings( dls );
    }

    // will be called after every testfunction.
    void cleanup()
    {

    }

    void testPieDiagram()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "\"Pilots\"" ) << QStringLiteral( "\"Cabin Crew\"" );
      ds.maxScaleDenominator = -1;
      ds.minScaleDenominator = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
      ds.size = QSizeF( 5, 5 );
      ds.angleOffset = 0;

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationField( "Staff" );
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );
      mPointsLayer->setDiagramRenderer( dr );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "piediagram" ) );
    }

    void testPieDiagramExpression()
    {
      QgsDiagramSettings ds;
      QColor col1 = Qt::red;
      QColor col2 = Qt::yellow;
      col1.setAlphaF( 0.5 );
      col2.setAlphaF( 0.5 );
      ds.categoryColors = QList<QColor>() << col1 << col2;
      ds.categoryAttributes = QList<QString>() << QStringLiteral( "ln(Pilots + 1)" ) << QStringLiteral( "ln(\"Cabin Crew\" + 1)" );
      ds.maxScaleDenominator = -1;
      ds.minScaleDenominator = -1;
      ds.minimumSize = 0;
      ds.penColor = Qt::green;
      ds.penWidth = .5;
      ds.scaleByArea = true;
      ds.sizeType = QgsUnitTypes::RenderMillimeters;
      ds.size = QSizeF( 5, 5 );
      ds.angleOffset = 0;

      QgsLinearlyInterpolatedDiagramRenderer *dr = new QgsLinearlyInterpolatedDiagramRenderer();
      dr->setLowerValue( 0.0 );
      dr->setLowerSize( QSizeF( 0.0, 0.0 ) );
      dr->setUpperValue( 10 );
      dr->setUpperSize( QSizeF( 40, 40 ) );
      dr->setClassificationAttributeIsExpression( true );
      dr->setClassificationAttributeExpression( QStringLiteral( "ln(Staff + 1)" ) );
      dr->setDiagram( new QgsPieDiagram() );
      dr->setDiagramSettings( ds );

      QgsDiagramLayerSettings dls = QgsDiagramLayerSettings();
      dls.setPlacement( QgsDiagramLayerSettings::OverPoint );
      dls.setShowAllDiagrams( true );
      // dls.setRenderer( dr );

      mPointsLayer->setDiagramRenderer( dr );
      mPointsLayer->setDiagramLayerSettings( dls );

      QVERIFY( imageCheck( "piediagram_expression" ) );

      mPointsLayer->setDiagramRenderer( 0 );
    }

};

bool TestQgsDiagram::imageCheck( const QString& theTestType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image

  QgsRectangle extent( -126, 23, -70, 47 );
  mMapSettings->setExtent( extent );
  mMapSettings->setFlag( QgsMapSettings::ForceVectorOutput );
  mMapSettings->setOutputDpi( 96 );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlName( "expected_" + theTestType );
  myChecker.setMapSettings( *mMapSettings );
  myChecker.setColorTolerance( 15 );
  bool myResultFlag = myChecker.runTest( theTestType, 200 );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsDiagram )
#include "testqgsdiagram.moc"
