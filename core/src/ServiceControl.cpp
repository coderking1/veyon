/*
 * ServiceControl.cpp - class for controlling veyon service
 *
 * Copyright (c) 2017 Tobias Junghans <tobydox@users.sf.net>
 *
 * This file is part of Veyon - http://veyon.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QProgressBar>
#include <QProgressDialog>
#include <QThread>

#include "VeyonCore.h"
#include "VeyonConfiguration.h"
#include "PlatformCoreFunctions.h"
#include "PlatformServiceFunctions.h"
#include "ServiceControl.h"


ServiceControl::ServiceControl(QWidget *parent) :
	QObject( parent ),
	m_serviceName( QStringLiteral( "VeyonService" ) ),
	m_parent( parent )
{
}



QString ServiceControl::serviceFilePath()
{
	return QDir::toNativeSeparators(
				QCoreApplication::applicationDirPath() + QDir::separator() +
				"veyon-service" + VeyonCore::platform().coreFunctions().programFileExtension() );
}



bool ServiceControl::isServiceRegistered()
{
	return VeyonCore::platform().serviceFunctions().isRegistered( m_serviceName );
}



bool ServiceControl::isServiceRunning()
{
	return VeyonCore::platform().serviceFunctions().isRunning( m_serviceName );
}



void ServiceControl::startService()
{
	serviceControl( tr( "Starting %1 Service" ).arg( VeyonCore::applicationName() ), { "-startservice" } );
}




void ServiceControl::stopService()
{
	serviceControl( tr( "Stopping %1 Service" ).arg( VeyonCore::applicationName() ), { "-stopservice" } );
}



void ServiceControl::registerService()
{
	serviceControl( tr( "Registering %1 Service" ).arg( VeyonCore::applicationName() ), { "-registerservice" } );
}



void ServiceControl::unregisterService()
{
	serviceControl( tr( "Unregistering %1 Service" ).arg( VeyonCore::applicationName() ), { "-unregisterservice" } );
}



void ServiceControl::serviceControl( const QString& title, QStringList arguments )
{
	// not running as graphical/interactive application?
	if( m_parent == nullptr )
	{
		// then prevent service application from showing any message boxes
		arguments.prepend( QStringLiteral( "-quiet" ) );
	}

	QProcess serviceProcess;
	serviceProcess.start( serviceFilePath(), arguments );
	serviceProcess.waitForStarted();

	if( m_parent )
	{
		graphicalFeedback( title, serviceProcess);
	}
	else
	{
		textFeedback( title, serviceProcess );
	}
}



void ServiceControl::graphicalFeedback( const QString &title, const QProcess& serviceProcess )
{
	QProgressDialog pd( title, QString(), 0, 0, m_parent );
	pd.setWindowTitle( VeyonCore::applicationName() );

	auto b = new QProgressBar( &pd );
	b->setMaximum( 100 );
	b->setTextVisible( false );
	pd.setBar( b );
	b->show();
	pd.setWindowModality( Qt::WindowModal );
	pd.show();

	int j = 0;
	while( serviceProcess.state() == QProcess::Running )
	{
		QCoreApplication::processEvents();
		b->setValue( ++j % 100 );
		QThread::msleep( 10 );
	}
}



void ServiceControl::textFeedback( const QString& title, const QProcess& serviceProcess )
{
	printf( "%s", qUtf8Printable( title ) );

	while( serviceProcess.state() == QProcess::Running )
	{
		QCoreApplication::processEvents();
		QThread::msleep( 200 );
		printf( "." );
	}
}
