/*
 * AuthKeysConfigurationPage.cpp - implementation of the authentication configuration page
 *
 * Copyright (c) 2017-2018 Tobias Junghans <tobydox@users.sf.net>
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

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#include "AuthKeysConfigurationPage.h"
#include "AuthKeysManager.h"
#include "FileSystemBrowser.h"
#include "VeyonConfiguration.h"
#include "Configuration/UiMapping.h"

#include "ui_AuthKeysConfigurationPage.h"


AuthKeysConfigurationPage::AuthKeysConfigurationPage() :
	ConfigurationPage(),
	ui(new Ui::AuthKeysConfigurationPage),
	m_keyListModel( this )
{
	ui->setupUi(this);

#define CONNECT_BUTTON_SLOT(name) \
			connect( ui->name, &QAbstractButton::clicked, this, &AuthKeysConfigurationPage::name );

	CONNECT_BUTTON_SLOT( openPublicKeyBaseDir );
	CONNECT_BUTTON_SLOT( openPrivateKeyBaseDir );
	CONNECT_BUTTON_SLOT( createKeyPair );
	CONNECT_BUTTON_SLOT( deleteKey );
	CONNECT_BUTTON_SLOT( importKey );
	CONNECT_BUTTON_SLOT( exportKey );

	ui->keyList->setModel( &m_keyListModel );

	reloadKeyList();
}


AuthKeysConfigurationPage::~AuthKeysConfigurationPage()
{
	delete ui;
}



void AuthKeysConfigurationPage::resetWidgets()
{
	FOREACH_VEYON_KEY_AUTHENTICATION_CONFIG_PROPERTY(INIT_WIDGET_FROM_PROPERTY);
}



void AuthKeysConfigurationPage::connectWidgetsToProperties()
{
	FOREACH_VEYON_KEY_AUTHENTICATION_CONFIG_PROPERTY(CONNECT_WIDGET_TO_PROPERTY);
}



void AuthKeysConfigurationPage::applyConfiguration()
{
}



void AuthKeysConfigurationPage::openPublicKeyBaseDir()
{
	FileSystemBrowser( FileSystemBrowser::ExistingDirectory ).
												exec( ui->publicKeyBaseDir );
}



void AuthKeysConfigurationPage::openPrivateKeyBaseDir()
{
	FileSystemBrowser( FileSystemBrowser::ExistingDirectory ).
			exec( ui->privateKeyBaseDir );
}



void AuthKeysConfigurationPage::createKeyPair()
{
	const auto keyName = QInputDialog::getText( this, tr( "Authentication key name" ),
												tr( "Please enter the name of the user group or role for which to create an authentication key pair:") );
	if( keyName.isEmpty() == false )
	{
		AuthKeysManager authKeysManager;
		const auto success = authKeysManager.createKeyPair( keyName );

		showResultMessage( success, tr( "Create key pair" ), authKeysManager.resultMessage() );

		reloadKeyList();
	}
}



void AuthKeysConfigurationPage::deleteKey()
{
	const auto title = tr( "Delete authentication key" );

	const auto nameAndType = selectedKey().split('/');

	if( nameAndType.size() > 1 )
	{
		const auto name = nameAndType[0];
		const auto type = nameAndType[1];

		if( QMessageBox::question( this, title, tr( "Do you really want to delete authentication key \"%1/%2\"?" ).arg( name, type ) ) )
		{
			AuthKeysManager authKeysManager;
			const auto success = authKeysManager.deleteKey( name, type );

			showResultMessage( success, title, authKeysManager.resultMessage() );

			reloadKeyList();
		}
	}
	else
	{
		showResultMessage( false, title, tr( "Please select a key to delete!" ) );
	}
}



void AuthKeysConfigurationPage::importKey()
{
	const auto title = tr( "Import authentication key" );

	const auto inputFile = QFileDialog::getOpenFileName( this, title );
	if( inputFile.isEmpty() )
	{
		return;
	}

	const auto keyName = QInputDialog::getText( this, tr( "Authentication key name" ),
												tr( "Please enter the name of the user group or role for which to import the authentication key:") );
	if( keyName.isEmpty() )
	{
		return;
	}

	AuthKeysManager authKeysManager;
	const auto keyType = authKeysManager.detectKeyType( inputFile );
	const auto success = authKeysManager.importKey( keyName, keyType, inputFile );

	showResultMessage( success, title, authKeysManager.resultMessage() );

	reloadKeyList();
}



void AuthKeysConfigurationPage::exportKey()
{
	const auto title = tr( "Export authentication key" );

	const auto nameAndType = selectedKey().split('/');

	if( nameAndType.size() > 1 )
	{
		const auto name = nameAndType[0];
		const auto type = nameAndType[1];

		const auto outputFile = QFileDialog::getSaveFileName( this, title, QDir::homePath() + QDir::separator() +
															  QStringLiteral("%1_%2_key.pem").arg( name, type ) );
		if( outputFile.isEmpty() == false )
		{
			AuthKeysManager authKeysManager;
			const auto success = authKeysManager.exportKey( name, type, outputFile );

			showResultMessage( success, title, authKeysManager.resultMessage() );
		}
	}
	else
	{
		showResultMessage( false, title, tr( "Please select a key to export!" ) );
	}
}



void AuthKeysConfigurationPage::reloadKeyList()
{
	m_keyListModel.setStringList( AuthKeysManager().listKeys() );
}



QString AuthKeysConfigurationPage::selectedKey() const
{
	const auto currentIndex = ui->keyList->currentIndex();

	return m_keyListModel.data( currentIndex, Qt::DisplayRole ).toString();
}



void AuthKeysConfigurationPage::showResultMessage( bool success, const QString& title, const QString& message )
{
	if( message.isEmpty() )
	{
		return;
	}

	if( success )
	{
		QMessageBox::information( this, title, message );
	}
	else
	{
		QMessageBox::critical( this, title, message );
	}
}