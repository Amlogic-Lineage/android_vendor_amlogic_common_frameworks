/*
 * Copyright (c) 2019 Hardkernel Co., Ltd. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: C++ file
 */

#define LOG_TAG "SystemControl"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "common.h"
#include "Ubootenv.h"

const char *PROFIX_UBOOTENV_VAR = "ubootenv.var.";

struct {
	const char *key;
	const char *rep;
} env_quirk[] = {
	{ "vout", "outputmode" },
};

const char *env_key_quirk(const char *key)
{
	int i;

	for (i = 0; i < sizeof(env_quirk) / sizeof(env_quirk[0]); i++) {
		if (!strcmp(key, env_quirk[i].key))
			return env_quirk[i].rep;
	}
	return key;
}

Ubootenv::Ubootenv() :
	mEnvInitDone(false),
	mEnvLock(PTHREAD_MUTEX_INITIALIZER)
{
	init();

	printValues();
}

Ubootenv::~Ubootenv()
{
	if (mEnvData.image) {
		free(mEnvData.image);
		mEnvData.image = NULL;
		mEnvData.crc = NULL;
		mEnvData.data = NULL;
	}
	env_attribute * pAttr = mEnvAttrHeader.next;
	memset(&mEnvAttrHeader, 0, sizeof(env_attribute));
	env_attribute * pTmp = NULL;
	while (pAttr) {
		pTmp = pAttr;
		pAttr = pAttr->next;
		free(pTmp);
	}
}

int Ubootenv::updateValue(const char* name, const char* value)
{
	SYS_LOGI("[ubootenv] -- %s (%d) is not implemented\n", __func__, __LINE__);
	return 0;
}

const char * Ubootenv::getValue(const char * key)
{
    if (!isEnv(key)) {
        //should assert here.
        SYS_LOGE("[ubootenv] %s is not a ubootenv varible.\n", key);
        return NULL;
    }

    mutex_lock(&mEnvLock);
    const char* envName = key + strlen(PROFIX_UBOOTENV_VAR);
    const char* envValue = get(envName);
    mutex_unlock(&mEnvLock);
    return envValue;
}

void Ubootenv::printValues()
{
	env_attribute *attr;

	for (attr = &mEnvAttrHeader; attr; attr = attr->next)
		SYS_LOGI("[ubootenv] key(%s) value(%s)", attr->key, attr->value);

}

int Ubootenv::reInit()
{
	SYS_LOGI("[ubootenv] -- %s (%d) is not implemented\n", __func__, __LINE__);
	return 0;
}

int Ubootenv::init()
{
	env_attribute *attr;

	mEnvPartitionSize = 0x10000;

	char *buf = (char*)malloc(mEnvPartitionSize);
	if (!buf)
		return -ENOMEM;

	memset(buf, 0, mEnvPartitionSize);

	mEnvData.data = buf;

	int fd = open("/proc/cmdline", O_RDONLY);
	if (fd < 0) {
		SYS_LOGE("[ubootenv] failed to access (%s)\n", strerror(errno));
		return -errno;
	}
	int siz = read(fd, buf, mEnvPartitionSize);
	if (siz <= 0) {
		SYS_LOGE("[ubootenv] failed to read (%s)\n", strerror(errno));
		close(fd);
		return -errno;
	}
	close(fd);

	*(char *)(buf + siz - 1) = '\0';

	pthread_mutex_lock(&mEnvLock);
	attr = parseAttribute();
	pthread_mutex_unlock(&mEnvLock);

	if (!attr)
		return -EINVAL;

	printValues();

	mEnvInitDone = true;

	return 0;
}

int Ubootenv::readPartitionData()
{
	SYS_LOGI("[ubootenv] -- %s (%d) is not implemented\n", __func__, __LINE__);
	return 0;
}

env_attribute* Ubootenv::parseAttribute()
{
	char *data = mEnvData.data;

	env_attribute_t *attr = &mEnvAttrHeader;

	SYS_LOGI("[ubootenv] env data=[%s]\n", data);

	memset(attr, 0, sizeof(env_attribute_t));

	/* Split string to tokens */
	char *token = strtok(data, " ");
	while (token) {
		char *p = strstr(token, "=");
		if (p) {
			*(char *)p = '\0';
			const char *key = env_key_quirk(token);
			char *val = p + 1;

			/* FIXME: take the first value in the values */
			p = strstr(val, ",");
			if (p)
				*(char*)p = '\0';

			strcpy(attr->key, key);
			strcpy(attr->value, val);

			attr->next = (env_attribute_t *)malloc(sizeof(env_attribute_t));
			if (!attr->next)
				break;
			memset(attr->next, 0, sizeof(env_attribute_t));
			attr = attr->next;
		}

		token = strtok(NULL, " ");
	}

	return &mEnvAttrHeader;
}

char * Ubootenv::get(const char * key)
{
	if (!mEnvInitDone) {
		SYS_LOGE("[ubootenv] don't init done\n");
		return NULL;
	}

	env_attribute *attr = &mEnvAttrHeader;
	while (attr) {
		if (!strcmp(key, attr->key)) {
			return attr->value;
		}
		attr = attr->next;
	}
	return NULL;
}

int Ubootenv::set(const char * key,  const char * value, bool createNew)
{
	SYS_LOGI("[ubootenv] -- %s (%d) is not implemented\n", __func__, __LINE__);
	return 0;
}

int Ubootenv::save()
{
	SYS_LOGI("[ubootenv] -- %s (%d) is not implemented\n", __func__, __LINE__);
	return 0;
}

int Ubootenv::formatAttribute()
{
	SYS_LOGI("[ubootenv] -- %s (%d) is not implemented\n", __func__, __LINE__);
	return 0;
}

int Ubootenv::isEnv(const char* prop_name)
{
	if (!prop_name || !(*prop_name))
		return 0;

	if (!(*PROFIX_UBOOTENV_VAR))
		return 0;

	if (strncmp(prop_name, PROFIX_UBOOTENV_VAR, strlen(PROFIX_UBOOTENV_VAR)) == 0
			&& strlen(prop_name) > strlen(PROFIX_UBOOTENV_VAR) )
		return 1;

	return 0;
}

void Ubootenv::propertyLoad()
{
	int count = 0;

	SYS_LOGI("[ubootenv] set property count: %d\n", count);
	mEnvInitDone = true;
}