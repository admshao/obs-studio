#include "obsremote-config.hpp"

OBSRemoteConfig *OBSRemoteConfig::_instance = new OBSRemoteConfig();

OBSRemoteConfig::OBSRemoteConfig()
{
	mbedtls_entropy_init(&entropy);
	mbedtls_ctr_drbg_init(&rng);
	mbedtls_ctr_drbg_seed(&rng, mbedtls_entropy_func, &entropy, nullptr, 0);
	mbedtls_ctr_drbg_set_prediction_resistance(&rng,
	                                           MBEDTLS_CTR_DRBG_PR_ON);

	const char *tmp = getChallenge();
	challenge = string(tmp);
	bfree((void *) tmp);
}

OBSRemoteConfig::~OBSRemoteConfig()
{
	mbedtls_ctr_drbg_free(&rng);
	mbedtls_entropy_free(&entropy);
}

const char *OBSRemoteConfig::getChallenge()
{
	unsigned char *random_chars = (unsigned char *) bmalloc(32);
	mbedtls_ctr_drbg_random(&rng, random_chars, 32);

	unsigned char *salt = (unsigned char *) bmalloc(64);
	size_t salt_bytes = 64;
	mbedtls_base64_encode(salt, 64, &salt_bytes, random_chars, 32);
	salt[salt_bytes] = 0;

	bfree(random_chars);
	return (char *) salt;
}

bool OBSRemoteConfig::UseAuth()
{
	return !password.empty();
}

void OBSRemoteConfig::SetPassword(string newpass)
{
	if (password.compare(newpass) == 0)
		return;

	if (newpass.empty()) {
		secret = "";
		salt = "";
		password = "";
	} else {
		password = newpass;

		const char *salt64 = getChallenge();
		size_t salt64Size = strlen(salt64);
		size_t passLength = password.size();

		size_t saltPlusPassSize = salt64Size + passLength;

		unsigned char *saltPlusPass = (unsigned char *) bmalloc(
			saltPlusPassSize);

		memcpy(saltPlusPass, password.c_str(), passLength);
		memcpy(saltPlusPass + passLength, salt64, salt64Size);

		unsigned char *passHash = (unsigned char *) bmalloc(32);
		unsigned char *passHash64 = (unsigned char *) bmalloc(
			64);
		size_t passHash64Size = 64;

		mbedtls_sha256(saltPlusPass, saltPlusPassSize, passHash,
		               0);

		memset(saltPlusPass, 0, saltPlusPassSize);

		mbedtls_base64_encode(passHash64, passHash64Size,
		                      &passHash64Size,
		                      passHash,
		                      32);
		passHash64[passHash64Size] = 0;

		salt = string(salt64);
		secret = string((const char *) passHash64);

		bfree(saltPlusPass);
		bfree(passHash);
		bfree((void *) salt64);
		bfree(passHash64);
	}
}

bool OBSRemoteConfig::Auth(const char *response)
{
	size_t challengeLength = challenge.size();
	size_t authHashLength = secret.size();

	size_t authPlusChallengeSize = authHashLength + challengeLength;
	unsigned char *authPlusChallenge = (unsigned char *) bmalloc
		(authPlusChallengeSize);

	memcpy(authPlusChallenge, secret.c_str(), authHashLength);
	memcpy(authPlusChallenge + authHashLength, challenge.c_str(),
	       challengeLength);

	unsigned char *respHash = (unsigned char *) bmalloc(32);
	unsigned char *respHash64 = (unsigned char *) bmalloc(64);
	size_t respHash64Size = 64;

	mbedtls_sha256(authPlusChallenge, authPlusChallengeSize, respHash, 0);
	mbedtls_base64_encode(respHash64, respHash64Size, &respHash64Size,
	                      respHash,
	                      32);
	respHash64[respHash64Size] = 0;

	bfree(authPlusChallenge);

	bool result = strcmp((char *) respHash64, response) == 0;

	bfree(respHash);
	bfree(respHash64);

	return result;
}

OBSRemoteConfig *OBSRemoteConfig::GetInstance()
{
	return _instance;
}
