#include <stdio.h>

#include "SSLSettings.h"
#include "TrustNETCASClient.h"
#include "CertManager.h"

int TrustNet_Init(void **ppContext, char * szConfPath)
{
	int nRet = 0;	

	if((nRet = CM_InitContext((Client_CTX **)ppContext)) != 0)
	{
		goto err;
	}

	if((nRet = CM_SetConfig((Client_CTX *)*ppContext, szConfPath)) != 0)
	{
		goto err;
	}

	if((nRet = CM_InitComm((Client_CTX *)*ppContext)) != 0)
	{
		goto err;
	}

err:

	return (nRet == 0 ? nRet : (nRet += ER_INIT));
}

int TrustNet_IssueDeviceCert(void *pContext, char *szDevId, int nReIssue)
{
	int nRet = 0;	
	char *pszDN = NULL;
	char *pszKeyAlgo = NULL;
	char *pszKeyLength = NULL;
	char *pszPass = NULL;
	char *pszCert = NULL;
	char *pszPriKey = NULL;
	Client_CTX *pCtx = (Client_CTX *)pContext;

	if((nRet = CM_CheckContext(pCtx)) != 0)
		goto err;

	// ������ ���� �˻�, ��ȿ�� �˻�
	if( nReIssue || CM_VerifyCert(szDevId) != 0 )
	{
		if((nRet = CM_RegDeviceGetDN(pCtx, szDevId, &pszDN, &pszKeyAlgo, &pszKeyLength)) != 0)
			goto err;

		if((nRet = CM_MakeCertPass(szDevId, &pszPass)) != 0)
			goto err;

		if((nRet = CM_IssueCertSimple(pCtx, szDevId, pszDN, pszKeyAlgo, pszKeyLength, pszPass, &pszCert, &pszPriKey)) != 0)
			goto err;

		if((nRet = CM_SaveCertSet(szDevId, pszCert, pszPriKey)) != 0)
			goto err;
	}
	
err:

	CM_MemFree((void**)&pszDN);
	CM_MemFree((void**)&pszKeyAlgo);
	CM_MemFree((void**)&pszKeyLength);
	CM_MemFree((void**)&pszPass);
	CM_MemFree((void**)&pszCert);
	CM_MemFree((void**)&pszPriKey);

	return (nRet == 0 ? nRet : (nRet += ER_ISSUEDEVICECERT));
}

int TrustNet_GetDeviceCert(void *pContext, char *szDevId, char **ppCert)
{
	int nRet = 0;
	Client_CTX * pCtx = (Client_CTX *)pContext;

	if((nRet = CM_GetCertSet(szDevId, ppCert, NULL)) != 0)
		goto err;

err:

	return (nRet == 0 ? nRet : (nRet += ER_GETDEVICECERT));
}

int TrustNet_GetDevicePrikeyAndPass(void *pContext, char *szDevId, char **ppPriKey, char **ppPass)
{
	int nRet = 0;
	Client_CTX * pCtx = (Client_CTX *)pContext;

	// ������ �˻� �� �� �߱� ���� ȣ��
	if((nRet =TrustNet_IssueDeviceCert(pContext, szDevId, CERT_ISSUE)) != 0)
		goto err;

	if((nRet = CM_GetCertSet(szDevId, NULL, ppPriKey)) != 0)
		goto err;

	if((nRet = CM_MakeCertPass(szDevId, ppPass)) != 0)
		goto err;
		
err:

	return (nRet == 0 ? nRet : (nRet += ER_GETDEVICEPRIKEYANDPASS));
}

int TrustNet_GetAuthKeyFromMEF(void *pContext, char *szDevId, char *szSignSrc, char *szSignValue, char **ppAuthKey)
{
	int nRet = 0;
	char *szSignCert = NULL;
	Client_CTX * pCtx = (Client_CTX *)pContext;

	if((nRet = CM_CheckContext(pCtx)) != 0)
		goto err;

// 	if((nRet = CM_CheckDeviceID(pCtx, szDevId)) != 0)
// 		goto err;

	if((nRet = CM_GetCertSet(szDevId, &szSignCert, NULL)) != 0)
		goto err;

	if((nRet =CM_AuthByCert(pCtx, szSignSrc, szSignValue, szSignCert, ppAuthKey)) != 0)
		goto err;

err:

	CM_MemFree((void**)&szSignCert);

	return (nRet == 0 ? nRet : (nRet += ER_GETAUTHKEYFROMMEF));
}

int TrustNet_Final(void **ppContext)
{
	int nRet = 0;
	Client_CTX * pCtx = (Client_CTX *)(*ppContext);	

	CM_FinalComm(pCtx);

	return CM_FinalContext(&pCtx);
}

#ifdef SIGN_TEST
int TrustNet_SignTest(char *signSrc, char *szPemPriKey, char *szPass, char **szSign)
{
	return sign(signSrc,szPemPriKey,szPass,szSign);
}


int TrustNet_VerifyTest(char *signSrc, char *szSign, char *szCert)
{
	return verify(signSrc, szSign, szCert);
}
#endif
