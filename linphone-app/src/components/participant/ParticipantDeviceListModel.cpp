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

#include <QQmlApplicationEngine>
#include <algorithm>

#include "app/App.hpp"

#include "ParticipantDeviceListModel.hpp"
#include "utils/Utils.hpp"

#include "components/Components.hpp"

// =============================================================================

ParticipantDeviceListModel::ParticipantDeviceListModel (std::shared_ptr<linphone::Participant> participant, QObject *parent) : ProxyListModel(parent) {
	std::list<std::shared_ptr<linphone::ParticipantDevice>> devices = participant->getDevices() ;
	mCallModel = nullptr;
	for(auto device : devices){
		auto deviceModel = ParticipantDeviceModel::create(mCallModel, device, isMe(device));
		connect(this, &ParticipantDeviceListModel::securityLevelChanged, deviceModel.get(), &ParticipantDeviceModel::onSecurityLevelChanged);
		connect(deviceModel.get(), &ParticipantDeviceModel::isSpeakingChanged, this, &ParticipantDeviceListModel::onParticipantDeviceSpeaking);
		mList << deviceModel;
	}
}

ParticipantDeviceListModel::ParticipantDeviceListModel (CallModel * callModel, QObject *parent) : ProxyListModel(parent) {
	if(callModel && callModel->isConference()) {
		mCallModel = callModel;
		auto conferenceModel = callModel->getConferenceSharedModel();
		std::list<std::shared_ptr<linphone::ParticipantDevice>> devices = conferenceModel->getConference()->getParticipantDeviceList();
		for(auto device : devices){
			auto deviceModel = ParticipantDeviceModel::create(mCallModel, device, isMe(device));
			connect(this, &ParticipantDeviceListModel::securityLevelChanged, deviceModel.get(), &ParticipantDeviceModel::onSecurityLevelChanged);
			connect(deviceModel.get(), &ParticipantDeviceModel::isSpeakingChanged, this, &ParticipantDeviceListModel::onParticipantDeviceSpeaking);
			mList << deviceModel;
		}
		connect(conferenceModel.get(), &ConferenceModel::participantAdded, this, &ParticipantDeviceListModel::onParticipantAdded);
		connect(conferenceModel.get(), &ConferenceModel::participantRemoved, this, &ParticipantDeviceListModel::onParticipantRemoved);
		connect(conferenceModel.get(), &ConferenceModel::participantDeviceAdded, this, &ParticipantDeviceListModel::onParticipantDeviceAdded);
		connect(conferenceModel.get(), &ConferenceModel::participantDeviceRemoved, this, &ParticipantDeviceListModel::onParticipantDeviceRemoved);
		connect(conferenceModel.get(), &ConferenceModel::participantDeviceJoined, this, &ParticipantDeviceListModel::onParticipantDeviceJoined);
		connect(conferenceModel.get(), &ConferenceModel::participantDeviceLeft, this, &ParticipantDeviceListModel::onParticipantDeviceLeft);
		connect(conferenceModel.get(), &ConferenceModel::conferenceStateChanged, this, &ParticipantDeviceListModel::onConferenceStateChanged);
		connect(conferenceModel.get(), &ConferenceModel::participantDeviceMediaCapabilityChanged, this, &ParticipantDeviceListModel::onParticipantDeviceMediaCapabilityChanged);
		connect(conferenceModel.get(), &ConferenceModel::participantDeviceMediaAvailabilityChanged, this, &ParticipantDeviceListModel::onParticipantDeviceMediaAvailabilityChanged);
		connect(conferenceModel.get(), &ConferenceModel::participantDeviceIsSpeakingChanged, this, &ParticipantDeviceListModel::onParticipantDeviceIsSpeakingChanged);
	}
}

void ParticipantDeviceListModel::updateDevices(std::shared_ptr<linphone::Participant> participant){
	std::list<std::shared_ptr<linphone::ParticipantDevice>> devices = participant->getDevices() ;
	beginResetModel();
	qDebug() << "Update devices from participant";
	mList.clear();
	for(auto device : devices){
		auto deviceModel = ParticipantDeviceModel::create(mCallModel, device, isMe(device));
		connect(this, &ParticipantDeviceListModel::securityLevelChanged, deviceModel.get(), &ParticipantDeviceModel::onSecurityLevelChanged);
		connect(deviceModel.get(), &ParticipantDeviceModel::isSpeakingChanged, this, &ParticipantDeviceListModel::onParticipantDeviceSpeaking);
		mList << deviceModel;
	}
	endResetModel();
}

void ParticipantDeviceListModel::updateDevices(const std::list<std::shared_ptr<linphone::ParticipantDevice>>& devices, const bool& isMe){
}

bool ParticipantDeviceListModel::add(std::shared_ptr<linphone::ParticipantDevice> deviceToAdd){
	qDebug() << "Adding device " << deviceToAdd->getAddress()->asString().c_str();
	for(auto item : mList) {
		auto deviceModel = item.objectCast<ParticipantDeviceModel>();
		if(deviceModel->getDevice() == deviceToAdd) {
			qWarning() << "Device already exist. Send video update event";
			deviceModel->updateVideoEnabled();
			return false;
		}
	}
	
	auto deviceModel = ParticipantDeviceModel::create(mCallModel, deviceToAdd, isMe(deviceToAdd));
	connect(this, &ParticipantDeviceListModel::securityLevelChanged, deviceModel.get(), &ParticipantDeviceModel::onSecurityLevelChanged);
	connect(deviceModel.get(), &ParticipantDeviceModel::isSpeakingChanged, this, &ParticipantDeviceListModel::onParticipantDeviceSpeaking);
	ProxyListModel::add<ParticipantDeviceModel>(deviceModel);
	qDebug() << "Device added. Count=" << mList.count();
	return true;
}

bool ParticipantDeviceListModel::remove(std::shared_ptr<const linphone::ParticipantDevice> deviceToRemove){
	int row = 0;
	for(auto item : mList){
		auto device = item.objectCast<ParticipantDeviceModel>();
		if( device->getDevice() == deviceToRemove){
			device->updateVideoEnabled();
			removeRow(row);
			return true;
		}else
			++row;
	}
	return false;
}

QSharedPointer<ParticipantDeviceModel> ParticipantDeviceListModel::get(std::shared_ptr<const linphone::ParticipantDevice> deviceToGet, int * index){
	int row = 0;
	for(auto item : mList){
		auto device = item.objectCast<ParticipantDeviceModel>();
		if( device->getDevice() == deviceToGet){
			if(index)
				*index = row;
			return device;
		}else
			++row;
	}
	return nullptr;
}

bool ParticipantDeviceListModel::isMe(std::shared_ptr<linphone::ParticipantDevice> deviceToCheck)const{
	if(mCallModel){
		auto devices = mCallModel->getConferenceSharedModel()->getConference()->getMe()->getDevices();
		auto deviceToCheckAddress = deviceToCheck->getAddress();
		for(auto device : devices){
			if(deviceToCheckAddress == device->getAddress())
				return true;
		}
	}
	return false;
}

bool ParticipantDeviceListModel::isMeAlone() const{
	for(auto item : mList){
		auto device = item.objectCast<ParticipantDeviceModel>();
		if( !isMe(device->getDevice()))
			return false;
	}
	return true;
}

void ParticipantDeviceListModel::onSecurityLevelChanged(std::shared_ptr<const linphone::Address> device){
	emit securityLevelChanged(device);
}

//----------------------------------------------------------------------------------------------------------
void ParticipantDeviceListModel::onParticipantAdded(const std::shared_ptr<const linphone::Participant> & participant){	
	std::list<std::shared_ptr<linphone::ParticipantDevice>> devices = participant->getDevices() ;
	if(devices.size() == 0)
		qWarning() << "Participant has no device. It will not be added : " << participant->getAddress()->asString().c_str();
	else
		for(auto device : devices)
			add(device);
}

void ParticipantDeviceListModel::onParticipantRemoved(const std::shared_ptr<const linphone::Participant> & participant){
	std::list<std::shared_ptr<linphone::ParticipantDevice>> devices = participant->getDevices() ;
	for(auto device : devices)
		remove(device);
}

void ParticipantDeviceListModel::onParticipantDeviceAdded(const std::shared_ptr<const linphone::ParticipantDevice> & participantDevice){
	qDebug() << "Adding new device : " << mList.count();
	auto conferenceModel = mCallModel->getConferenceSharedModel();
	std::list<std::shared_ptr<linphone::ParticipantDevice>> devices = conferenceModel->getConference()->getParticipantDeviceList();
	for(auto realParticipantDevice : devices){
		if( realParticipantDevice == participantDevice){
			add(realParticipantDevice);
			return;
		}
	}
	qWarning() << "No participant device found from linphone::ParticipantDevice at onParticipantDeviceAdded";
}

void ParticipantDeviceListModel::onParticipantDeviceRemoved(const std::shared_ptr<const linphone::ParticipantDevice> & participantDevice){
	qDebug() << "Removing participant device : " << mList.count();
	if(!remove(participantDevice))
		qWarning() << "No participant device found from linphone::ParticipantDevice at onParticipantDeviceRemoved";
}

void ParticipantDeviceListModel::onParticipantDeviceJoined(const std::shared_ptr<const linphone::ParticipantDevice> & participantDevice){
	for(auto item : mList) {
		auto device = item.objectCast<ParticipantDeviceModel>();
		if(device->getDevice() == participantDevice) {
			device->setPaused(false);
			return;
		}
	}
	onParticipantDeviceAdded(participantDevice);
}

void ParticipantDeviceListModel::onParticipantDeviceLeft(const std::shared_ptr<const linphone::ParticipantDevice> & participantDevice){
	for(auto item : mList) {
		auto device = item.objectCast<ParticipantDeviceModel>();
		if(device->getDevice() == participantDevice) {
			device->setPaused(true);
			return;
		}
	}
}

void ParticipantDeviceListModel::onConferenceStateChanged(linphone::Conference::State newState){
	if(newState == linphone::Conference::State::Created){
		if(mCallModel && mCallModel->isConference()) {
			auto conferenceModel = mCallModel->getConferenceSharedModel();
			updateDevices(conferenceModel->getConference()->getMe()->getDevices(), true);
			updateDevices(conferenceModel->getConference()->getParticipantDeviceList(), false);
		}
	}
}

void ParticipantDeviceListModel::onParticipantDeviceMediaCapabilityChanged(const std::shared_ptr<const linphone::ParticipantDevice> & participantDevice){
	auto device = get(participantDevice);
	if(device)
		device->updateVideoEnabled();
	else
		onParticipantDeviceAdded(participantDevice);
		
	device = get(participantDevice);
	if( device && device->isMe()){	// Capability change for me. Update all videos.
		for(auto item : mList) {
			auto device = item.objectCast<ParticipantDeviceModel>();
			device->updateVideoEnabled();
		}
	}
}

void ParticipantDeviceListModel::onParticipantDeviceMediaAvailabilityChanged(const std::shared_ptr<const linphone::ParticipantDevice> & participantDevice){
	auto device = get(participantDevice);
	if(device)
		device->updateVideoEnabled();
	else
		onParticipantDeviceAdded(participantDevice);
}

void ParticipantDeviceListModel::onParticipantDeviceIsSpeakingChanged(const std::shared_ptr<const linphone::ParticipantDevice> & participantDevice, bool isSpeaking){
	if( isSpeaking){
		auto device = get(participantDevice);
		if( device)
			emit participantSpeaking(device.get());
	}
}

void ParticipantDeviceListModel::onParticipantDeviceSpeaking(){
	emit participantSpeaking(qobject_cast<ParticipantDeviceModel*>(sender()));
}
