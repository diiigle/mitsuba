/*
    This file is part of Mitsuba, a physically based rendering system.

    Copyright (c) 2007-2014 by Wenzel Jakob and others.

    Mitsuba is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Mitsuba is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <mitsuba/render/volume.h>

#if defined(__LINUX__)
#include <sys/mman.h>
#include <fcntl.h>
#endif

MTS_NAMESPACE_BEGIN

VolumeDataSource::VolumeDataSource(Stream *stream, InstanceManager *manager) :
    ConfigurableObject(stream, manager) {
    m_aabb = AABB(stream);
}

VolumeDataSource::VolumeDataSource(const Properties &props) : ConfigurableObject(props) { }

VolumeDataSource::~VolumeDataSource() { }

void VolumeDataSource::serialize(Stream *stream, InstanceManager *manager) const {
    ConfigurableObject::serialize(stream, manager);
    m_aabb.serialize(stream);
}

Float VolumeDataSource::lookupFloat(const Point &p) const {
    Log(EError, "'%s': does not implement lookupFloat()!", getClass()->getName().c_str());
    return 0;
}

Spectrum VolumeDataSource::lookupSpectrum(const Point &p) const {
    Log(EError, "'%s': does not implement lookupSpectrum()!", getClass()->getName().c_str());
    return Spectrum(0.0f);
}

Vector VolumeDataSource::lookupVector(const Point &p) const {
    Log(EError, "'%s': does not implement lookupVector()!", getClass()->getName().c_str());
    return Vector();
}

bool VolumeDataSource::supportsFloatLookups() const {
    return false;
}

bool VolumeDataSource::supportsSpectrumLookups() const {
    return false;
}

bool VolumeDataSource::supportsVectorLookups() const {
    return false;
}

void VolumeDataSource::editFloat(const Point &p, const Float &f) {
    Log(EError, "'%s': does not implement editFloat()!", getClass()->getName().c_str());
}

void VolumeDataSource::editSpectrum(const Point &p, const Spectrum &s) {
    Log(EError, "'%s': does not implement editSpectrum()!", getClass()->getName().c_str());
}

void VolumeDataSource::editVector(const Point &p, const Vector &v) {
    Log(EError, "'%s': does not implement editVector()!", getClass()->getName().c_str());
}

bool VolumeDataSource::supportsFloatEdits() const {
    return false;
}

bool VolumeDataSource::supportsSpectrumEdits() const {
    return false;
}

bool VolumeDataSource::supportsVectorEdits() const {
    return false;
}

MTS_IMPLEMENT_CLASS(VolumeDataSource, true, ConfigurableObject)
MTS_NAMESPACE_END

