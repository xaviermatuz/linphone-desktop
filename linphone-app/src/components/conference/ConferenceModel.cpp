/*
 * Copyright (c) 2021 Belledonne Communications SARL.
 *
 * This file is part of linphone-desktop
 * (see https://www.linphone.org).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ConferenceModel.hpp"
#include "ConferenceListener.hpp"

#include <QQmlApplicationEngine>
#include <QDesktopServices>
#include <QImageReader>
#include <QMessageBox>

#include "app/App.hpp"
#include "app/paths/Paths.hpp"
#include "app/providers/ThumbnailProvider.hpp"


#include "utils/QExifImageHeader.hpp"
#include "utils/Utils.hpp"
#include "utils/Constants.hpp"
#include "components/Components.hpp"

void ConferenceModel::connectTo(ConferenceListener * listener){
	connect(listener, &ConferenceListener::participantAdded, this, &ConferenceModel::onParticipantAdded);
	connect(listener, &ConferenceListener::participantRemoved, this, &ConferenceModel::onParticipantRemoved);
	connect(listener, &ConferenceListener::participantAdminStatusChanged, this, &ConferenceModel::onParticipantAdminStatusChanged);
	connect(listener, &ConferenceListener::participantDeviceAdded, this, &ConferenceModel::onParticipantDeviceAdded);
	connect(listener, &ConferenceListener::participantDeviceRemoved, this, &ConferenceModel::onParticipantDeviceRemoved);
	connect(listener, &ConferenceListener::participantDeviceLeft, this, &ConferenceModel::onParticipantDeviceLeft);
	connect(listener, &ConferenceListener::participantDeviceJoined, this, &ConferenceModel::onParticipantDeviceJoined);
	connect(listener, &ConferenceListener::participantDeviceMediaCapabilityChanged, this, &ConferenceModel::onParticipantDeviceMediaCapabilityChanged);
	connect(listener, &ConferenceListener::participantDeviceMediaAvailabilityChanged, this, &ConferenceModel::onParticipantDeviceMediaAvailabilityChanged);
	connect(listener, &ConferenceListener::participantDeviceIsSpeakingChanged, this, &ConferenceModel::onParticipantDeviceIsSpeakingChanged);
	connect(listener, &ConferenceListener::conferenceStateChanged, this, &ConferenceModel::onConferenceStateChanged);
	connect(listener, &ConferenceListener::subjectChanged, this, &ConferenceModel::onSubjectChanged);
}

// =============================================================================

QSharedPointer<ConferenceModel> ConferenceModel::create(std::shared_ptr<linphone::Conference> conference, QObject *parent){
	return QSharedPointer<ConferenceModel>::create(conference, parent);
}

ConferenceModel::ConferenceModel (std::shared_ptr<linphone::Conference> conference, QObject *parent) : QObject(parent) {
	App::getInstance()->getEngine()->setObjectOwnership(this, QQmlEngine::CppOwnership);// Avoid QML to destroy it when passing by Q_INVOKABLE
	mConference = conference;
	mParticipantListModel = QSharedPointer<ParticipantListModel>::create(this);
	mConferenceListener = std::make_shared<ConferenceListener>();
	connectTo(mConferenceListener.get());
	mConference->addListener(mConferenceListener);
}

ConferenceModel::~ConferenceModel(){
	mConference->removeListener(mConferenceListener);
}

bool ConferenceModel::updateLocalParticipant(){
	bool changed = false;
	// First try to use findParticipant
	auto localParticipant = mConference->findParticipant(mConference->getCall()->getCallLog()->getLocalAddress());
	// Me is not in participants, use Me().
	if( !localParticipant)
		localParticipant = mConference->getMe();
	if( localParticipant){
		mLocalParticipant = QSharedPointer<ParticipantModel>::create(localParticipant);
		qDebug() << "Is Admin: " << localParticipant->isAdmin() << " " << mLocalParticipant->getAdminStatus();
		changed = true;
	}
	return changed;
}

std::shared_ptr<linphone::Conference> ConferenceModel::getConference()const{
	return mConference;
}

QString ConferenceModel::getSubject() const{
	return QString::fromStdString(mConference->getSubject());
}

QDateTime ConferenceModel::getStartDate() const{
	return QDateTime::fromSecsSinceEpoch(mConference->getStartTime());
}

qint64 ConferenceModel::getElapsedSeconds() const {
	return getStartDate().secsTo(QDateTime::currentDateTime());
}

ParticipantModel* ConferenceModel::getLocalParticipant() const{
	if( mLocalParticipant) {
		qDebug() << "LocalParticipant admin : " << mLocalParticipant->getAdminStatus() << " " << (mLocalParticipant->getParticipant() ? mLocalParticipant->getParticipant()->isAdmin() : -1);
	}else
		qDebug() << "No LocalParticipant";
	return mLocalParticipant.get();
}

ParticipantListModel* ConferenceModel::getParticipantListModel() const{
	return mParticipantListModel.get();
}

std::list<std::shared_ptr<linphone::Participant>> ConferenceModel::getParticipantList()const{
	auto participantList = mConference->getParticipantList();
	auto me = mConference->getMe();
	if( me )
		participantList.push_front(me);
	return participantList;
}
//-----------------------------------------------------------------------------------------------------------------------
//												LINPHONE LISTENERS
//-----------------------------------------------------------------------------------------------------------------------
void ConferenceModel::onParticipantAdded(const std::shared_ptr<const linphone::Participant> & participant){
	qDebug() << "Added call, participant count: " << getParticipantList().size();
	if(!mLocalParticipant){
		if(updateLocalParticipant())
			emit localParticipantChanged();
	}
	emit participantAdded(participant);
}
void ConferenceModel::onParticipantRemoved(const std::shared_ptr<const linphone::Participant> & participant){
	qDebug() << "Me devices : " << mConference->getMe()->getDevices().size();
	emit participantRemoved(participant);
}
void ConferenceModel::onParticipantAdminStatusChanged(const std::shared_ptr<const linphone::Participant> & participant){
	qDebug() << "onParticipantAdminStatusChanged: " << participant->getAddress()->asString().c_str();
	if(participant == mLocalParticipant->getParticipant())
		emit mLocalParticipant->adminStatusChanged();
	emit participantAdminStatusChanged(participant);
}

void ConferenceModel::onParticipantDeviceAdded(const std::shared_ptr<const linphone::ParticipantDevice> & participantDevice){
	qDebug() << "Me devices : " << mConference->getMe()->getDevices().size();
	emit participantDeviceAdded(participantDevice);
}
void ConferenceModel::onParticipantDeviceRemoved(const std::shared_ptr<const linphone::ParticipantDevice> & participantDevice){
	qDebug() << "Me devices : " << mConference->getMe()->getDevices().size();
	emit participantDeviceRemoved(participantDevice);
}
void ConferenceModel::onParticipantDeviceLeft(const std::shared_ptr<const linphone::ParticipantDevice> & participantDevice){
	qDebug() << "Me devices : " << mConference->getMe()->getDevices().size();
	emit participantDeviceLeft(participantDevice);
}
void ConferenceModel::onParticipantDeviceJoined(const std::shared_ptr<const linphone::ParticipantDevice> & participantDevice){
	qDebug() << "Me devices : " << mConference->getMe()->getDevices().size();
	emit participantDeviceJoined(participantDevice);
}
void ConferenceModel::onParticipantDeviceMediaCapabilityChanged(const std::shared_ptr<const linphone::ParticipantDevice> & participantDevice){
	qDebug() << "ConferenceModel::onParticipantDeviceMediaCapabilityChanged: "  << (int)participantDevice->getStreamCapability(linphone::StreamType::Video) << ". Me devices : " << mConference->getMe()->getDevices().size();
	emit participantDeviceMediaCapabilityChanged(participantDevice);
}
void ConferenceModel::onParticipantDeviceMediaAvailabilityChanged(const std::shared_ptr<const linphone::ParticipantDevice> & participantDevice){
	qDebug() << "ConferenceModel::onParticipantDeviceMediaAvailabilityChanged: "  << (int)participantDevice->getStreamAvailability(linphone::StreamType::Video) << ". Me devices : " << mConference->getMe()->getDevices().size();
	emit participantDeviceMediaAvailabilityChanged(participantDevice);
}
void ConferenceModel::onParticipantDeviceIsSpeakingChanged(const std::shared_ptr<const linphone::ParticipantDevice> & participantDevice, bool isSpeaking){
	emit participantDeviceIsSpeakingChanged(participantDevice, isSpeaking);
}
void ConferenceModel::onConferenceStateChanged(linphone::Conference::State newState){
	emit conferenceStateChanged(newState);
}
void ConferenceModel::onSubjectChanged(const std::string& string){
	emit subjectChanged();
}
	
	
//-----------------------------------------------------------------------------------------------------------------------	
