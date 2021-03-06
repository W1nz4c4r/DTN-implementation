/*
 *	libcfdp.c:	functions enabling the utilization of CFDP
 *			services.
 *
 *	Copyright (c) 2009, California Institute of Technology.
 *	ALL RIGHTS RESERVED.  U.S. Government Sponsorship acknowledged.
 *
 *	Author: Scott Burleigh, JPL
 *
 */

#include "cfdpP.h"
#include "dirent.h"

#ifndef NO_PROXY

extern void	parseProxyPutRequest(char *text, int bytesRemaining,
			CfdpUserOpsData *opsData);
extern void	parseProxyMsgToUser(char *text, int bytesRemaining,
			CfdpUserOpsData *opsData);
extern void	parseProxyFilestoreRequest(char *text, int bytesRemaining,
			CfdpUserOpsData *opsData);
extern void	parseProxyFaultHandlerOverride(char *text, int bytesRemaining,
			CfdpUserOpsData *opsData);
extern void	parseProxyTransmissionMode(char *text, int bytesRemaining,
			CfdpUserOpsData *opsData);
extern void	parseProxyFlowLabel(char *text, int bytesRemaining,
			CfdpUserOpsData *opsData);
extern void	parseProxySegmentationControl(char *text, int bytesRemaining,
			CfdpUserOpsData *opsData);
extern void	parseProxyClosureRequest(char *text, int bytesRemaining,
			CfdpUserOpsData *opsData);
extern void	parseProxyPutResponse(char *text, int bytesRemaining,
			CfdpUserOpsData *opsData);
extern void	parseProxyFilestoreResponse(char *text, int bytesRemaining,
			CfdpUserOpsData *opsData);
extern int	handleProxyPutRequest(CfdpUserOpsData *opsData);
extern int	handleProxyPutCancel(CfdpUserOpsData *opsData);

#endif

extern void	parseOriginatingTransactionId(char *text, int bytesRemaining,
			CfdpUserOpsData *opsData);

#ifndef NO_DIRLIST

extern void	parseDirectoryListingRequest(char *text, int bytesRemaining,
			CfdpUserOpsData *opsData);
extern void	parseDirectoryListingResponse(char *text, int bytesRemaining,
			CfdpUserOpsData *opsData);
extern int	handleDirectoryListingRequest(CfdpUserOpsData *opsData);

#endif

int	cfdp_attach()
{
	return cfdpAttach();
}

int	cfdp_entity_is_started()
{
	CfdpVdb	*vdb = getCfdpVdb();

	return (vdb && vdb->clockPid != ERROR);
}

void	 cfdp_detach()
{
#if (!(defined (ION_LWT)))
	cfdpDetach();
#endif
	ionDetach();
}

void	cfdp_compress_number(CfdpNumber *nbr, uvast val)
{
	unsigned char	*octet;

	CHKVOID(nbr);
	nbr->length = 0;
	memset(nbr->buffer, 0, 8);
	octet = nbr->buffer + 7;	/*	Last byte of buffer.	*/
	while (val > 0)
	{
		*octet = val & 0xff;	/*	Copy lowest-order byte.	*/
		nbr->length++;
		if (nbr->length == 8)
		{
			return;		/*	Can't copy any more.	*/
		}

		octet--;		/*	Next-lowest-order byte.	*/

		/*	Can't just shift val 8 bits to the right
		 *	because we have no guarantee that the new
		 *	high-order 8 bits will be zero, and we can't
		 *	just "& 0x00ffff..." those high-order bits
		 *	because we don't know how many low-order
		 *	bytes there are (we might be on 32-bit
		 *	machine or on a 64-bit machine).  And we
		 *	can't memcpy a zero byte to the high-order
		 *	byte of val because that won't work the same
		 *	way on machines with different byte ordering.
		 *	A scalar divide is more expensive but is the
		 *	most portable solution.				*/

		val = val / 256;
	}

	if (nbr->length == 0)		/*	In case value is zero.	*/
	{
		nbr->length = 1;	/*	Minimum value length.	*/
	}
}

void	cfdp_decompress_number(uvast *val, CfdpNumber *nbr)
{
	unsigned char	*octet;
	vast		digit;
	int		i;

	CHKVOID(val);
	CHKVOID(nbr);
	*val = 0;
	for (i = 0, octet = nbr->buffer + 7; i < nbr->length; i++, octet--)
	{
		digit = *octet;
		digit <<= (i << 3);
		*val += digit;
	}
}

#ifndef ENABLE_HIGH_SPEED
void	cfdp_update_checksum(unsigned char octet, vast *offset,
		unsigned int *checksum, CfdpCksumType ckType)
{
	addToChecksum(octet, offset, checksum, ckType);
}
#endif

static int	defaultReader(int fd, unsigned int *checksum,
			CfdpCksumType ckType)
{
	static char	defaultReaderBuf[CFDP_MAX_PDU_SIZE];
	CfdpDB		*cfdpConstants = getCfdpConstants();
	vast		offset;
	int		length;
#ifndef ENABLE_HIGH_SPEED
	int		i;
	char		*octet;
#endif

	offset = ilseek(fd, 0, SEEK_CUR);
	if (offset < 0)
	{
		putSysErrmsg("CFDP can't get current file offset", NULL);
		return -1;
	}

	length = read(fd, defaultReaderBuf, cfdpConstants->maxFileDataLength);
	if (length < 0)
	{
		putSysErrmsg("CFDP default file reader failed", NULL);
		return -1;
	}

#ifdef ENABLE_HIGH_SPEED
	addDataToChecksum((unsigned char *) defaultReaderBuf, length, &offset, checksum, ckType);
#else
	for (i = 0, octet = defaultReaderBuf; i < length; i++, octet++)
	{
		addToChecksum((unsigned char) *octet, &offset, checksum,
				ckType);
	}
#endif

	return length;
}

int	cfdp_read_space_packets(int fd, unsigned int *checksum,
		CfdpCksumType ckType)
{
	static char	pktReaderBuf[CFDP_MAX_PDU_SIZE];
	CfdpDB		*cfdpConstants = getCfdpConstants();
	vast		offset;
	int		length;
#ifndef ENABLE_HIGH_SPEED
	int		i;
#endif
	char		*octet;
	unsigned int	recordLen;
	unsigned short	pktlen;

	offset = ilseek(fd, 0, SEEK_CUR);
	if (offset < 0)
	{
		putSysErrmsg("CFDP can't get current file offset", NULL);
		return -1;
	}

	length = read(fd, pktReaderBuf, cfdpConstants->maxFileDataLength);
	if (length < 0)
	{
		putSysErrmsg("CFDP space packet reader failed", NULL);
		return -1;
	}

	recordLen = 0;
	octet = pktReaderBuf;
	while (1)
	{
		if (length - recordLen < 7)
		{
			break;	/*	Too few bytes for a packet.	*/
		}

		/*	The current definition of a valid space
		 *	packet is a packet that has its first 4 bits
		 *	set to 0.					*/

		if ((*octet & 0xe0) != 0)
		{
			return 0;	/*	Not a packet.  Give up.	*/
		}

		/*	Get the length of the remaining packet from
		 *	bytes 4 and 5 of the packet, skipping bytes
		 *	0-3.  The packet length is only the length
		 *	of the secondary header and data portion and
		 *	does not include the length of the primary
		 *	header (6 bytes).  The length of the primary
		 *	header must be added back to the packet length,
		 *	and the result must be increased by 1 because
		 *	packet lengths are understated by 1 to enable
		 *	max length of 65536.				*/

		pktlen = 6	/*	Size of pkt primary header.	*/
			+ (((unsigned char) *(octet + 4)) << 8)
			+ (unsigned char) *(octet + 5)
			+ 1;	/*	Length in header is len - 1.	*/
		if (length - recordLen < pktlen)
		{
			break;	/*	Packet truncated in buffer.	*/
		}

		/*	Skip ahead to the next packet in the buffer.	*/

		recordLen += pktlen;
		octet += pktlen;
	}

	length = recordLen;

	/*	Now know the record length, so reposition to the
	 *	end of this record for next read.			*/

	if (ilseek(fd, offset + length, SEEK_SET) < 0)
	{
		putSysErrmsg("CFDP space packet reader failed", NULL);
		return -1;
	}

	/*	Add record to checksum.					*/

#ifdef ENABLE_HIGH_SPEED
	addDataToChecksum((unsigned char *) pktReaderBuf, length, &offset, checksum, ckType);
#else
	for (i = 0, octet = pktReaderBuf; i < length; i++, octet++)
	{
		addToChecksum((unsigned char) *octet, &offset, checksum,
				ckType);
	}
#endif

	return length;
}

int	cfdp_read_text_lines(int fd, unsigned int *checksum,
		CfdpCksumType ckType)
{
	static char	textReaderBuf[CFDP_MAX_PDU_SIZE];
	CfdpDB		*cfdpConstants = getCfdpConstants();
	vast		offset;
	int		length;
	int		i;
	char		*octet;

	offset = ilseek(fd, 0, SEEK_CUR);
	if (offset < 0)
	{
		putSysErrmsg("CFDP can't get current file offset", NULL);
		return -1;
	}

	length = read(fd, textReaderBuf, cfdpConstants->maxFileDataLength);
	if (length < 0)
	{
		putSysErrmsg("CFDP text line reader failed", NULL);
		return -1;
	}

	/*	Find last newline in buffer.  It's the last octet of
	 *	the record.						*/

	octet = textReaderBuf + (length - 1);
	i = length;
	while (i > 0)
	{
		if (*octet == '\n')
		{
			break;
		}

		i--;
	}

	if (i == 0)
	{
		return 0;	/*	No text lines.  Give up.	*/
	}

	length = i;

	/*	Now know the record length, so reposition to the
	 *	end of this record for next read.			*/

	if (ilseek(fd, offset + length, SEEK_SET) < 0)
	{
		putSysErrmsg("CFDP text line reader failed", NULL);
		return -1;
	}

	/*	Add record to checksum.					*/

#ifdef ENABLE_HIGH_SPEED
	addDataToChecksum((unsigned char *) textReaderBuf, length, &offset, checksum, ckType);
#else
	for (i = 0, octet = textReaderBuf; i < length; i++, octet++)
	{
		addToChecksum((unsigned char) *octet, &offset, checksum,
				ckType);
	}
#endif

	return length;
}

MetadataList	cfdp_create_usrmsg_list()
{
	return createMetadataList((getCfdpConstants())->usrmsgLists);
}

int	cfdp_add_usrmsg(MetadataList list, unsigned char *text, int length)
{
	CfdpDB		*cfdpConstants = getCfdpConstants();
	Sdr		sdr = getIonsdr();
	MsgToUser	usrmsg;
	Object		addr;

	CHKERR(list);
	CHKERR(text);
	CHKERR(length > 0);
	CHKERR(sdr_begin_xn(sdr));
	if (sdr_list_list(sdr, sdr_list_user_data(sdr, list))
			!= cfdpConstants->usrmsgLists)
	{
		sdr_exit_xn(sdr);
		putErrmsg("CFDP: error in list user data.", NULL);
		return -1;
	}

	if (length > 255)
	{
		sdr_exit_xn(sdr);
		putErrmsg("CFDP: User Message too long.", itoa(length));
		return -1;
	}

	memset((char *) &usrmsg, 0, sizeof(MsgToUser));
	usrmsg.length = length;
	usrmsg.text = sdr_malloc(sdr, length);
	if (usrmsg.text)
	{
		sdr_write(sdr, usrmsg.text, (char *) text, length);
		addr = sdr_malloc(sdr, sizeof(MsgToUser));
		if (addr)
		{
			sdr_write(sdr, addr, (char *) &usrmsg,
					sizeof(MsgToUser));
			oK(sdr_list_insert_last(sdr, list, addr));
		}
	}

	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("CFDP: failed adding user message.", NULL);
		return -1;
	}

	return 0;
}

int	cfdp_get_usrmsg(MetadataList *list, unsigned char *textBuf, int *length)
{
	Sdr		sdr = getIonsdr();
	Object		elt;
	Object		addr;
	MsgToUser	usrmsg;

	CHKERR(list);
	CHKERR(textBuf);
	CHKERR(length);
	*length = 0;			/*	Default.		*/
	*textBuf = '\0';		/*	Default.		*/
	if (*list == 0)			/*	Got end of list.	*/
	{
		return 0;
	}

	CHKERR(sdr_begin_xn(sdr));
	elt = sdr_list_first(sdr, *list);
	if (elt == 0)
	{
		destroyUsrmsgList(list);
		if (sdr_end_xn(sdr) < 0)
		{
			putErrmsg("CFDP: failed destroying usrmsg list.", NULL);
			return -1;
		}

		return 0;
	}

	addr = sdr_list_data(sdr, elt);
	sdr_read(sdr, (char *) &usrmsg, addr, sizeof(MsgToUser));
	*length = usrmsg.length;
	if (usrmsg.text)
	{
		sdr_read(sdr, (char *) textBuf, usrmsg.text, usrmsg.length);
		sdr_free(sdr, usrmsg.text);
	}

	sdr_free(sdr, addr);
	sdr_list_delete(sdr, elt, NULL, NULL);
	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("CFDP: failed getting user message.", NULL);
		return -1;
	}

	return 0;
}

void	cfdp_destroy_usrmsg_list(Object *list)
{
	Sdr	sdr = getIonsdr();

	CHKVOID(list && *list);
	CHKVOID(sdr_begin_xn(sdr));
	destroyUsrmsgList(list);
	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("CFDP: failed destroying usrmsg list.", NULL);
	}
}

MetadataList	cfdp_create_fsreq_list()
{
	return createMetadataList((getCfdpConstants())->fsreqLists);
}

int	cfdp_add_fsreq(MetadataList list, CfdpAction action,
		char *firstFileName, char *secondFileName)
{
	CfdpDB			*cfdpConstants = getCfdpConstants();
	Sdr			sdr = getIonsdr();
	FilestoreRequest	fsreq;
	Object			addr;

	CHKERR(list);
	CHKERR(firstFileName == NULL || strlen(firstFileName) < 256);
	CHKERR(secondFileName == NULL || strlen(secondFileName) < 256);
	CHKERR(sdr_begin_xn(sdr));
	if (sdr_list_list(sdr, sdr_list_user_data(sdr, list))
			!= cfdpConstants->fsreqLists)
	{
		sdr_exit_xn(sdr);
		putErrmsg("CFDP: error in list user data.", NULL);
		return -1;
	}

	memset((char *) &fsreq, 0, sizeof(FilestoreRequest));
	fsreq.action = action;
	if (firstFileName)
	{
		fsreq.firstFileName = sdr_string_create(sdr, firstFileName);
	}

	if (secondFileName)
	{
		fsreq.secondFileName = sdr_string_create(sdr,
				secondFileName);
	}

	addr = sdr_malloc(sdr, sizeof(FilestoreRequest));
	if (addr)
	{
		sdr_write(sdr, addr, (char *) &fsreq,
				sizeof(FilestoreRequest));
		oK(sdr_list_insert_last(sdr, list, addr));
	}

	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("CFDP: failed adding filestore request.", NULL);
		return -1;
	}

	return 0;
}

int	cfdp_get_fsreq(MetadataList *list, CfdpAction *action,
		char *firstFileNameBuf, char *secondFileNameBuf)
{
	Sdr			sdr = getIonsdr();
	Object			elt;
	Object			addr;
	FilestoreRequest	fsreq;

	CHKERR(list);
	CHKERR(action);
	CHKERR(firstFileNameBuf);
	CHKERR(secondFileNameBuf);
	*action = (CfdpAction) -1;	/*	Default.		*/
	*firstFileNameBuf = '\0';	/*	Default.		*/
	*secondFileNameBuf = '\0';	/*	Default.		*/
	if (*list == 0)
	{
		return 0;		/*	Got end of list.	*/
	}

	CHKERR(sdr_begin_xn(sdr));
	elt = sdr_list_first(sdr, *list);
	if (elt == 0)
	{
		destroyFsreqList(list);
		if (sdr_end_xn(sdr) < 0)
		{
			putErrmsg("CFDP: failed destroying fsreq list.", NULL);
			return -1;
		}

		return 0;
	}

	addr = sdr_list_data(sdr, elt);
	sdr_read(sdr, (char *) &fsreq, addr, sizeof(FilestoreRequest));
	*action = fsreq.action;
	if (fsreq.firstFileName)
	{
		sdr_string_read(sdr, firstFileNameBuf, fsreq.firstFileName);
		sdr_free(sdr, fsreq.firstFileName);
	}

	if (fsreq.secondFileName)
	{
		sdr_string_read(sdr, secondFileNameBuf, fsreq.secondFileName);
		sdr_free(sdr, fsreq.secondFileName);
	}

	sdr_free(sdr, addr);
	sdr_list_delete(sdr, elt, NULL, NULL);
	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("CFDP: failed getting filestore request.", NULL);
		return -1;
	}

	return 0;
}

void	cfdp_destroy_fsreq_list(Object *list)
{
	Sdr	sdr = getIonsdr();

	CHKVOID(list && *list);
	CHKVOID(sdr_begin_xn(sdr));
	destroyFsreqList(list);
	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("CFDP: failed destroying fsreq list.", NULL);
	}
}

int	cfdp_get_fsresp(MetadataList *list, CfdpAction *action,
		int *status, char *firstFileNameBuf, char *secondFileNameBuf,
		char *messageBuf)
{
	Sdr			sdr = getIonsdr();
	Object			elt;
	Object			addr;
	FilestoreResponse	fsresp;

	CHKERR(list);
	CHKERR(action);
	CHKERR(status);
	CHKERR(firstFileNameBuf);
	CHKERR(secondFileNameBuf);
	CHKERR(messageBuf);
	*action = (CfdpAction) -1;	/*	Default.		*/
	*status = -1;			/*	Default.		*/
	*firstFileNameBuf = '\0';	/*	Default.		*/
	*secondFileNameBuf = '\0';	/*	Default.		*/
	*messageBuf = '\0';		/*	Default.		*/
	if (*list == 0)
	{
		return 0;		/*	Got end of list.	*/
	}

	CHKERR(sdr_begin_xn(sdr));
	elt = sdr_list_first(sdr, *list);
	if (elt == 0)
	{
		destroyFsrespList(list);
		if (sdr_end_xn(sdr) < 0)
		{
			putErrmsg("CFDP: failed destroying fsresp list.", NULL);
		}

		return 0;
	}

	addr = sdr_list_data(sdr, elt);
	sdr_read(sdr, (char *) &fsresp, addr, sizeof(FilestoreResponse));
	*action = fsresp.action;
	*status = fsresp.status;
	if (fsresp.firstFileName)
	{
		sdr_string_read(sdr, firstFileNameBuf, fsresp.firstFileName);
		sdr_free(sdr, fsresp.firstFileName);
	}

	if (fsresp.secondFileName)
	{
		sdr_string_read(sdr, secondFileNameBuf, fsresp.secondFileName);
		sdr_free(sdr, fsresp.secondFileName);
	}

	if (fsresp.message)
	{
		sdr_string_read(sdr, messageBuf, fsresp.message);
		sdr_free(sdr, fsresp.message);
	}

	sdr_free(sdr, addr);
	sdr_list_delete(sdr, elt, NULL, NULL);
	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("CFDP: failed getting filestore response.", NULL);
		return -1;
	}

	return 0;
}

void	cfdp_destroy_fsresp_list(Object *list)
{
	Sdr	sdr = getIonsdr();

	CHKVOID(list && *list);
	CHKVOID(sdr_begin_xn(sdr));
	destroyFsrespList(list);
	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("CFDP: failed destroying fsresp list.", NULL);
	}
}

char	*cfdp_working_directory()
{
	return getIonWorkingDirectory();
}

/*	*	*	CFDP service functions	*	*	*	*/

static int	constructOriginatingXnIdMsg(CfdpTransactionId *transactionId,
			unsigned char *textBuffer)
{
	int	length;
	int	sourceEntityNbrPad;
	int	transactionNbrPad;

	/*	Construct the message.					*/

	length = 6;	/*	"cfdp" + msgtype + lengths byte		*/
	memcpy(textBuffer, "cfdp", 4);
	textBuffer += 4;
	*textBuffer = (unsigned char) CfdpOriginatingTransactionId;
	textBuffer++;
	length += transactionId->sourceEntityNbr.length;
	sourceEntityNbrPad = 8 - transactionId->sourceEntityNbr.length;
	length += transactionId->transactionNbr.length;
	transactionNbrPad = 8 - transactionId->transactionNbr.length;

	/*	Insert lengths byte value.				*/

	*textBuffer = ((transactionId->sourceEntityNbr.length - 1) << 4)
			+ (transactionId->transactionNbr.length - 1);
	textBuffer++;

	/*	Insert both transaction ID parameters.			*/

	memcpy(textBuffer, transactionId->sourceEntityNbr.buffer
		+ sourceEntityNbrPad, transactionId->sourceEntityNbr.length);
	textBuffer += transactionId->sourceEntityNbr.length;
	memcpy(textBuffer, transactionId->transactionNbr.buffer
		+ transactionNbrPad, transactionId->transactionNbr.length);
	return length;
}

static int	constructMetadataPdu(OutFdu *fdu, char *sourceFileName,
			char *destFileName, CfdpHandler *faultHandlers,
			unsigned int flowLabelLength, unsigned char *flowLabel,
			MetadataList messagesToUser,
			MetadataList filestoreRequests,
			CfdpTransactionId *originatingTransactionId)
{
	static unsigned char	mpduBuf[CFDP_MAX_PDU_SIZE];
	Sdr			sdr = getIonsdr();
	unsigned char		*cursor;
	unsigned int		mpduLength = 0;
	unsigned int		smallFileSize;
	uvast			largeFileSize;
	size_t			length;
	int			i;
	CfdpHandler		*override;
	Object			elt;
	Object			nextElt;
	Object			obj;
				OBJ_POINTER(FilestoreRequest, req);
				OBJ_POINTER(MsgToUser, msg);
	int			firstFileNameLen;
	char			firstFileName[256];
	int			secondFileNameLen;
	char			secondFileName[256];
	unsigned char		msgText[256];

	cursor = mpduBuf;

	/*	Insert directive code.					*/

	*cursor = 7;		/*	Metadata PDU.			*/
	cursor++;
	mpduLength++;

	/*	Note closure request and checksum type.			*/

	*cursor = (fdu->closureLatency > 0) << 6;
	*cursor += (fdu->ckType & 0x0f);
	cursor++;
	mpduLength++;

	/*	Note file size.						*/

	if (fdu->largeFile)
	{
		largeFileSize = fdu->fileSize;
		largeFileSize = htonv(largeFileSize);
		memcpy(cursor, (char *) &largeFileSize, 8);
		cursor += 8;
		mpduLength += 8;
	}
	else
	{
		smallFileSize = fdu->fileSize;
		smallFileSize = htonl(smallFileSize);
		memcpy(cursor, (char *) &smallFileSize, 4);
		cursor += 4;
		mpduLength += 4;
	}

	/*	Note source file name.					*/

	if (sourceFileName != NULL && destFileName != NULL
	&& strcmp(sourceFileName, destFileName) == 0)
	{
		/*	Don't bother to send source file name; it's
		 *	the same as the destination file name.		*/

		sourceFileName = NULL;
	}

	length = (sourceFileName == NULL ? 0 : strlen(sourceFileName));
	*cursor = length;
	cursor++;
	mpduLength++;
	if (length > 0)
	{
		memcpy(cursor, sourceFileName, length);
		cursor += length;
		mpduLength += length;
	}

	/*	Note destination file name.				*/

	length = (destFileName == NULL ? 0 : strlen(destFileName));
	*cursor = length;
	cursor++;
	mpduLength++;
	if (length > 0)
	{
		memcpy(cursor, destFileName, length);
		cursor += length;
		mpduLength += length;
	}

	/*	Note filestore request TLVs.				*/

	if (filestoreRequests != 0)
	{
		/*	Detach list from log in database.		*/

		sdr_list_delete(sdr, sdr_list_user_data(sdr,
				filestoreRequests), NULL, NULL);
		sdr_list_user_data_set(sdr, filestoreRequests, 0);

		/*	Process all requests in list.			*/

		for (elt = sdr_list_first(sdr, filestoreRequests); elt;
				elt = nextElt)
		{
			nextElt = sdr_list_next(sdr, elt);
			obj = sdr_list_data(sdr, elt);
			GET_OBJ_POINTER(sdr, FilestoreRequest, req, obj);
			firstFileNameLen = 0;
			firstFileName[0] = '\0';
			if (req->firstFileName)
			{
				firstFileNameLen = sdr_string_read(sdr,
					firstFileName, req->firstFileName);
				if (firstFileNameLen < 0)
				{
					putErrmsg("Missing sdrstring.", NULL);
					break;
				}

				sdr_free(sdr, req->firstFileName);
			}

			secondFileNameLen = 0;
			secondFileName[0] = '\0';
			if (req->secondFileName)
			{
				secondFileNameLen = sdr_string_read(sdr,
					secondFileName, req->secondFileName);
				if (secondFileNameLen < 0)
				{
					putErrmsg("Missing sdrstring.", NULL);
					break;
				}

				sdr_free(sdr, req->secondFileName);
			}

			sdr_list_delete(sdr, elt, NULL, NULL);
			sdr_free(sdr, obj);

			/*	Append to metadata if possible.		*/

			length = 1 + 1 + firstFileNameLen
					+ 1 + secondFileNameLen;
			if (length > 255)
			{
				putErrmsg("Filestore request too long.",
						itoa(length));
				break;
			}

			if (mpduLength + 1 + 1 + length > CFDP_MAX_PDU_SIZE)
			{
				putErrmsg("Metadata too long.", itoa(length));
				break;
			}

			*cursor = 0x00;			/*	Type.	*/
			cursor++;
			mpduLength++;
			*cursor = length;
			cursor++;
			mpduLength++;
			*cursor = (int) (req->action) << 4;
			cursor++;
			mpduLength++;
			*cursor = firstFileNameLen;
			cursor++;
			mpduLength++;
			memcpy(cursor, firstFileName, firstFileNameLen);
			cursor += firstFileNameLen;
			mpduLength += firstFileNameLen;
			*cursor = secondFileNameLen;
			cursor++;
			mpduLength++;
			memcpy(cursor, secondFileName, secondFileNameLen);
			cursor += secondFileNameLen;
			mpduLength += secondFileNameLen;
		}
	}

	/*	Note message-to-user TLVs.				*/

	if (messagesToUser != 0)
	{
		/*	Detach list from log in database.		*/

		sdr_list_delete(sdr, sdr_list_user_data(sdr, messagesToUser),
				NULL, NULL);
		sdr_list_user_data_set(sdr, messagesToUser, 0);

		/*	Process all messages in list.			*/

		for (elt = sdr_list_first(sdr, messagesToUser); elt;
				elt = nextElt)
		{
			nextElt = sdr_list_next(sdr, elt);
			obj = sdr_list_data(sdr, elt);
			GET_OBJ_POINTER(sdr, MsgToUser, msg, obj);
			length = (msg->text == 0 ? 0 : msg->length);
			if (length == 0)
			{
				msgText[0] = '\0';
			}
			else
			{
				sdr_read(sdr, (char *) msgText, msg->text,
						length);
				sdr_free(sdr, msg->text);
			}

			sdr_list_delete(sdr, elt, NULL, NULL);
			sdr_free(sdr, obj);

			/*	Append to metadata if possible.		*/

			if (mpduLength + 1 + 1 + length > CFDP_MAX_PDU_SIZE)
			{
				putErrmsg("Metadata too long.", itoa(length));
				break;
			}

			*cursor = 0x02;			/*	Type.	*/
			cursor++;
			mpduLength++;
			*cursor = length;
			cursor++;
			mpduLength++;
			memcpy(cursor, (char *) msgText, length);
			cursor += length;
			mpduLength += length;
		}
	}

	if (originatingTransactionId != NULL)
	{
		length = constructOriginatingXnIdMsg(originatingTransactionId,
				msgText);

		/*	Append to metadata if possible.			*/

		if (mpduLength + 1 + 1 + length > CFDP_MAX_PDU_SIZE)
		{
			putErrmsg("Metadata too long.", itoa(length));
		}
		else
		{
			*cursor = 0x02;			/*	Type.	*/
			cursor++;
			mpduLength++;
			*cursor = length;
			cursor++;
			mpduLength++;
			memcpy(cursor, (char *) msgText, length);
			cursor += length;
			mpduLength += length;
		}
	}

	/*	Note fault handler TLVs.				*/

	if (faultHandlers)
	{
		for (i = 0, override = faultHandlers; i < 16; i++, override++)
		{
			if (*override != CfdpNoHandler)
			{
				length = 1;

				/*	Append to metadata if possible.	*/

				if (mpduLength + 1 + 1 + length >
						CFDP_MAX_PDU_SIZE)
				{
					putErrmsg("Metadata too long.",
							itoa(length));
					break;
				}

				*cursor = 0x04;		/*	Type.	*/
				cursor++;
				mpduLength++;
				*cursor = length;
				cursor++;
				mpduLength++;
				*cursor = (i << 4) + ((int) (*override));
				cursor++;
				mpduLength++;
			}
		}
	}

	/*	Note flow label TLV.					*/

	if (flowLabel == NULL || flowLabelLength > 255)
	{
		flowLabelLength = 0;
	}

	if (flowLabelLength > 0)
	{
		length = flowLabelLength;

		/*	Append to metadata if possible.			*/

		if (mpduLength + 1 + 1 + length > CFDP_MAX_PDU_SIZE)
		{
			putErrmsg("Metadata too long.", itoa(length));
		}
		else
		{
			*cursor = 0x05;			/*	Type.	*/
			cursor++;
			mpduLength++;
			*cursor = length;
			cursor++;
			mpduLength++;
			memcpy(cursor, flowLabel, length);
			cursor += length;
			mpduLength += length;
		}
	}

	fdu->metadataPdu = sdr_malloc(sdr, mpduLength);
	if (fdu->metadataPdu == 0)
	{
		putErrmsg("Can't construct Metadata PDU.", NULL);
		return -1;
	}

	sdr_write(sdr, fdu->metadataPdu, (char *) mpduBuf, mpduLength);
	fdu->mpduLength = mpduLength;
	return 0;
}

static int	constructEofPdu(OutFdu *fdu, CfdpDB *db, unsigned int checksum)
{
	Sdr		sdr = getIonsdr();
	unsigned char	eofBuf[14];
	unsigned char	*cursor;
	unsigned int	epduLength = 0;
	unsigned char	condition = CfdpNoError;
	unsigned int	u4;
	uvast		u8;

	cursor = eofBuf;
	*cursor = 4;		/*	Directive code: EOF PDU.	*/
	cursor++;
	epduLength++;

	/*	The EOF is constructed immediately, as soon as the
	 *	Put request is received.  So its condition code must
	 *	be No Error, because there hasn't been an opportunity
	 *	to cancel or suspend the FDU.  EOF condition codes
	 *	other than No Error are useful only for Acknowledged-
	 *	mode FDUs.						*/

	*cursor = (condition << 4);
	cursor++;
	epduLength++;
	u4 = htonl(checksum);
	memcpy(cursor, (char *) &u4, 4);
	cursor += 4;
	epduLength += 4;

	/*	Note file size.						*/

	if (fdu->largeFile)
	{
		u8 = fdu->fileSize;
		u8 = htonv(u8);
		memcpy(cursor, (char *) &u8, 8);
		cursor += 8;
		epduLength += 8;
	}
	else
	{
		u4 = fdu->fileSize;
		u4 = htonl(u4);
		memcpy(cursor, (char *) &u4, 4);
		cursor += 4;
		epduLength += 4;
	}

	fdu->eofPdu = sdr_malloc(sdr, epduLength);
	if (fdu->eofPdu == 0)
	{
		putErrmsg("Can't construct EOF PDU.", NULL);
		return -1;
	}

	sdr_write(sdr, fdu->eofPdu, (char *) eofBuf, epduLength);
	fdu->epduLength = epduLength;
	return 0;
}

int	createFDU(CfdpNumber *destinationEntityNbr, unsigned int utParmsLength,
		unsigned char *utParms, char *sourceFileName,
		char *destFileName, CfdpReaderFn readerFn,
		CfdpMetadataFn metadataFn, CfdpHandler *faultHandlers,
		int flowLabelLength, unsigned char *flowLabel,
		unsigned int closureLatency, MetadataList messagesToUser,
		MetadataList filestoreRequests,
		CfdpTransactionId *originatingTransactionId,
		CfdpTransactionId *transactionId)
{
	CfdpVdb		*vdb = getCfdpVdb();
	Sdr		sdr = getIonsdr();
	Object		dbObj = getCfdpDbObject();
	OutFdu		fdu;
	uvast		destinationEntityId;
	CfdpDB		db;
	Object		elt;
			OBJ_POINTER(Entity, entity);
	CfdpHandler	handler;
	char		sourceFileBuf[MAXPATHLEN + 1];
	int		sourceFile;
	vast		fileSize;
	unsigned int	truncatedFileSize;
	vast		progress;
	int		recordLength;
	unsigned int	checksum = 0;
	Object		pduObj;
	FileDataPdu	pdu;
	int		lengthRemaining;
	char		metadataBuffer[255];
	Object		fduObj;
	CfdpEvent	event;
	int		metadataFnRet;

	CHKZERO(transactionId);
	memset((char *) transactionId, 0, sizeof(CfdpTransactionId));
	CHKZERO(destinationEntityNbr
	&& destinationEntityNbr->length > 0
	&& destinationEntityNbr->length < 9);
	if (utParmsLength == 0)
	{
		CHKZERO(utParms == NULL);
	}
	else
	{
		CHKZERO(utParms);
		CHKZERO(utParmsLength <= sizeof fdu.utParms);
	}

	CHKZERO(flowLabelLength >= 0 && flowLabelLength < 256);
	if (flowLabelLength == 0)
	{
		CHKZERO(flowLabel == NULL);
	}
	else
	{
		CHKZERO(flowLabel);
	}

	cfdp_decompress_number(&destinationEntityId, destinationEntityNbr);
	memset((char *) &fdu, 0, sizeof(OutFdu));
	if (utParms)
	{
		fdu.utParmsLength = utParmsLength;
		memcpy(fdu.utParms, utParms, utParmsLength);
	}
	else
	{
		fdu.utParmsLength = 0;
	}

	fdu.closureLatency = closureLatency;
	CHKZERO(sdr_begin_xn(sdr));

	/*	We build the FDU in SDR heap space, so we check here
	 *	to make sure there's a good chance that this will
	 *	succeed.  This is completely separate from the
	 *	check for insertion into Outbound ZCO space that
	 *	implements flow control.				*/

	if (sdr_heap_depleted(sdr))
	{
		sdr_exit_xn(sdr);
		writeMemoNote("[?] Low on heap space, can't send FDU.",
				sourceFileName);
		return 0;
	}

#ifdef ENABLE_HIGH_SPEED
	fdu.ckType = CRC32CChecksum;	/*	Default if high speed.	*/
#else
	fdu.ckType = ModularChecksum;	/*	Default.		*/
#endif
	sdr_stage(sdr, (char *) &db, dbObj, sizeof(CfdpDB));
	for (elt = sdr_list_first(sdr, db.entities); elt;
			elt = sdr_list_next(sdr, elt))
	{
		GET_OBJ_POINTER(sdr, Entity, entity, sdr_list_data(sdr, elt));
		if (entity->entityId == destinationEntityId)
		{
			if (ckTypeOkay(entity->outCkType))
			{
				fdu.ckType = entity->outCkType;
			}
			else
			{
				if (handleFault(&fdu.transactionId,
					CfdpUnsupportedChecksumType,
					&handler) < 0)
				{
					putErrmsg("No fault handler.", NULL);
				}
			}

			break;
		}
	}

	if (sourceFileName == NULL)
	{
		if (destFileName != NULL)
		{
			sdr_exit_xn(sdr);
			writeMemoNote("[?] CFDP: dest file name s/b NULL.",
					destFileName);
			return 0;
		}
	}
	else	/*	Construct contents of file data PDUs.		*/
	{
		if (qualifyFileName(sourceFileName, sourceFileBuf,
				sizeof sourceFileBuf) < 0)
		{
			sdr_exit_xn(sdr);
			putErrmsg("Source file name unusable: length.",
					sourceFileName);
			return 0;
		}

		if (strlen(sourceFileBuf) >= 256)
		{
			sdr_exit_xn(sdr);
			writeMemoNote("[?] CFDP: source file name too long.",
					sourceFileBuf);
			return 0;
		}

		if (destFileName == NULL)
		{
			destFileName = sourceFileName;
		}

		if (strlen(destFileName) >= 256)
		{
			sdr_exit_xn(sdr);
			writeMemoNote("[?] CFDP: dest file name too long.",
					destFileName);
			return 0;
		}

		sourceFileName = sourceFileBuf;
		if (checkFile(sourceFileName) != 1)
		{
			sdr_exit_xn(sdr);
			writeMemoNote("[?] CFDP can't find source file.",
					sourceFileName);
			return 0;
		}

		istrcpy(fdu.sourceFileName, sourceFileName,
				sizeof(fdu.sourceFileName));
		if (readerFn == NULL)
		{
			fdu.recordBoundsRespected = 0;
			readerFn = defaultReader;
		}
		else
		{
			fdu.recordBoundsRespected = 1;
		}

		sourceFile = ifopen(sourceFileName, O_RDONLY, 0);
		if (sourceFile < 0)
		{
			sdr_exit_xn(sdr);
			putSysErrmsg("CFDP can't open source file",
					sourceFileName);
			return 0;
		}

		fileSize = ilseek(sourceFile, 0, SEEK_END);
		if (fileSize < 0 || ilseek(sourceFile, 0, SEEK_SET) < 0)
		{
			close(sourceFile);
			sdr_exit_xn(sdr);
			putSysErrmsg("CFDP can't get fileSize", sourceFileName);
			return -1;
		}

		fdu.fileSize = fileSize;
		truncatedFileSize = fileSize;
		if (truncatedFileSize == fileSize)
		{
			fdu.largeFile = 0;
		}
		else
		{
			fdu.largeFile = 1;
		}

		fdu.fileDataPdus = sdr_list_create(sdr);
		if (fdu.fileDataPdus == 0)
		{
			close(sourceFile);
			sdr_cancel_xn(sdr);
			putErrmsg("CFDP can't create list of lengths.", NULL);
			return -1;
		}

		progress = 0;
		while (1)
		{
			recordLength = readerFn(sourceFile, &checksum,
					fdu.ckType);
			if (recordLength < 0)
			{
				close(sourceFile);
				sdr_cancel_xn(sdr);
				putErrmsg("CFDP failed reading records.",
						sourceFileName);
				return -1;
			}

			if (recordLength == 0)
			{
				if (ilseek(sourceFile, 0, SEEK_CUR)
						== fdu.fileSize)
				{
					break;	/*	No more records.*/
				}

				/*	Stopped before end of file.	*/

				if (handleFault(&fdu.transactionId,
						CfdpInvalidFileStructure,
						&handler) < 0)
				{
					close(sourceFile);
					sdr_cancel_xn(sdr);
					putErrmsg("Can't segment file.", NULL);
					return -1;
				}

				switch (handler)
				{
				case CfdpCancel:
				case CfdpAbandon:
					close(sourceFile);
					sdr_cancel_xn(sdr);
					return 0;	/*	Done.	*/

				default:	/*	No problem.	*/
					break;	/*	Out of switch.	*/
				}

				break;		/*	No more records.*/
			}

			/*	Note parameters of this PDU.		*/

			pduObj = sdr_malloc(sdr, sizeof(FileDataPdu));
			if (pduObj == 0)
			{
				close(sourceFile);
				sdr_cancel_xn(sdr);
				putErrmsg("CFDP failed creating file data PDU.",
						sourceFileName);
				return -1;
			}

			lengthRemaining = recordLength;
			while (lengthRemaining > 0)
			{
				pdu.offset = progress;
				if (lengthRemaining > db.maxFileDataLength)
				{
					pdu.length = db.maxFileDataLength;
					if (lengthRemaining == recordLength)
					{
						pdu.continuationState =
							CfdpStartOfRecord;
					}
					else
					{
						pdu.continuationState =
							CfdpNoBoundary;
					}
				}
				else
				{
					pdu.length = lengthRemaining;
					if (lengthRemaining == recordLength)
					{
						if (fdu.recordBoundsRespected)
						{
							pdu.continuationState =
							CfdpEntireRecord;
						}
						else
						{
							pdu.continuationState =
							CfdpNoBoundary;
						}
					}
					else
					{
						pdu.continuationState =
							CfdpEndOfRecord;
					}
				}

				if (metadataFn)
				{
					metadataFnRet =
						metadataFn(progress,
						recordLength - lengthRemaining,
						pdu.length, sourceFile,
						metadataBuffer);
					if (metadataFnRet < 0)
					{
						close(sourceFile);
						sdr_cancel_xn(sdr);
						putErrmsg("CFDP metadataFn \
failed.", sourceFileName);
						return -1;
					}

					if (metadataFnRet > 63)
					{
						close(sourceFile);
						sdr_cancel_xn(sdr);
						putErrmsg("CFDP seg metadata \
too long.", sourceFileName);
						return -1;
					}

					pdu.metadataLength =
						(unsigned int) metadataFnRet;
				}
				else
				{
					pdu.metadataLength = 0;
				}

				if (pdu.metadataLength == 0)
				{
					pdu.metadata = 0;
				}
				else
				{
					pdu.metadata = sdr_malloc(sdr,
							pdu.metadataLength);
					if (pdu.metadata == 0)
					{
						close(sourceFile);
						sdr_cancel_xn(sdr);
						putErrmsg("CFDP seg metadata.",
								sourceFileName);
						return -1;
					}

					sdr_write(sdr, pdu.metadata,
							metadataBuffer,
							pdu.metadataLength);
				}

				if (sdr_list_insert_last(sdr, fdu.fileDataPdus,
						pduObj) == 0)
				{
					close(sourceFile);
					sdr_cancel_xn(sdr);
					putErrmsg("CFDP file data PDUs failed.",
							sourceFileName);
					return -1;
				}

				sdr_write(sdr, pduObj, (char *) &pdu,
						sizeof(FileDataPdu));
				progress += pdu.length;
				lengthRemaining -= pdu.length;
			}
		}

		close(sourceFile);
	}

	/*	Prepare for construction of PDUs for this FDU.		*/

	fdu.reqNbr = getReqNbr();
	if (originatingTransactionId)
	{
		memcpy((char *) &fdu.originatingTransactionId,
				(char *) originatingTransactionId,
				sizeof(CfdpTransactionId));
	}

	/*	Construct the Metadata PDU.				*/

	if (constructMetadataPdu(&fdu, sourceFileName, destFileName,
			faultHandlers, flowLabelLength, flowLabel,
			messagesToUser, filestoreRequests,
			originatingTransactionId) < 0)
	{
		sdr_cancel_xn(sdr);
		putErrmsg("CFDP can't construct Metadata PDU.", NULL);
		return -1;
	}

	if (fdu.metadataPdu == 0)
	{
		sdr_cancel_xn(sdr);
		putErrmsg("CFDP can't construct Metadata PDU.", NULL);
		return 0;
	}

	/*	Construct the EOF PDU.					*/

	if (constructEofPdu(&fdu, &db, checksum) < 0)
	{
		sdr_cancel_xn(sdr);
		putErrmsg("CFDP can't construct EOF PDU.", NULL);
		return -1;
	}

	if (fdu.eofPdu == 0)
	{
		sdr_cancel_xn(sdr);
		putErrmsg("CFDP can't construct EOF PDU.", NULL);
		return 0;
	}

	memcpy((char *) &fdu.transactionId.sourceEntityNbr,
			(char *) &db.ownEntityNbr, sizeof(CfdpNumber));
	db.transactionCounter++;
	if (db.transactionCounter > db.maxTransactionNbr)
	{
		db.transactionCounter = 1;
	}

	sdr_write(sdr, dbObj, (char *) &db, sizeof(CfdpDB));
	cfdp_compress_number(&fdu.transactionId.transactionNbr,
			db.transactionCounter);
	memcpy((char *) &fdu.destinationEntityNbr,
			(char *) destinationEntityNbr, sizeof(CfdpNumber));
	fdu.state = FduActive;
	fduObj = sdr_malloc(sdr, sizeof(OutFdu));
	if (fduObj == 0)
	{
		sdr_cancel_xn(sdr);
		putErrmsg("No space for CFDP outbound FDU.", sourceFileName);
		return -1;
	}

	sdr_write(sdr, fduObj, (char *) &fdu, sizeof(OutFdu));
	sdr_list_insert_last(sdr, db.outboundFdus, fduObj);
	if (messagesToUser)
	{
		destroyUsrmsgList(&messagesToUser);
	}

	if (filestoreRequests)
	{
		destroyFsreqList(&filestoreRequests);
	}

	/*	Post Transaction.ind event.				*/

	memset((char *) &event, 0, sizeof(CfdpEvent));
	event.type = CfdpTransactionInd;
	memcpy((char *) &event.transactionId, (char *) &fdu.transactionId,
			sizeof(CfdpTransactionId));
	memcpy((char *) transactionId, (char *) &fdu.transactionId,
				sizeof(CfdpTransactionId));
	event.reqNbr = fdu.reqNbr;
	if (enqueueCfdpEvent(&event) < 0)
	{
		putErrmsg("CFDP can't report on new transaction.", NULL);
		sdr_cancel_xn(sdr);
		return -1;
	}

	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("CFDP can't create outbound FDU.", NULL);
		return -1;
	}

	sm_SemGive(vdb->fduSemaphore);
	return fdu.reqNbr;
}

int	cfdp_put(CfdpNumber *destinationEntityNbr, unsigned int utParmsLength,
		unsigned char *utParms, char *sourceFileName,
		char *destFileName, CfdpReaderFn readerFn,
		CfdpMetadataFn metadataFn, CfdpHandler *faultHandlers,
		unsigned int flowLabelLength, unsigned char *flowLabel,
		unsigned int closureLatency, Object messagesToUser,
		Object filestoreRequests, CfdpTransactionId *transactionId)
{
	return createFDU(destinationEntityNbr, utParmsLength, utParms,
			sourceFileName, destFileName, readerFn, metadataFn,
			faultHandlers, flowLabelLength, flowLabel,
			closureLatency, messagesToUser, filestoreRequests,
			NULL, transactionId);
}

int	cfdp_cancel(CfdpTransactionId *transactionId)
{
	Sdr	sdr = getIonsdr();
	CfdpDB	*db = getCfdpConstants();
	int	reqNbr;
	Object	fduObj;
	InFdu	fduBuf;
	Object	fduElt;

	CHKERR(transactionId);
	CHKERR(transactionId->sourceEntityNbr.length);
	CHKERR(transactionId->transactionNbr.length);
	CHKERR(sdr_begin_xn(sdr));
	if (memcmp(transactionId->sourceEntityNbr.buffer,
			db->ownEntityNbr.buffer, 8) == 0)
	{
		reqNbr = getReqNbr();
		if (cancelOutFdu(transactionId, CfdpCancelRequested,
					reqNbr) < 0)
		{
			putErrmsg("CFDP can't cancel outbound transaction.",
					NULL);
			sdr_cancel_xn(sdr);
			return -1;
		}
	}
	else	/*	Cancel inbound FDU.				*/
	{
		fduObj = findInFdu(transactionId, &fduBuf, &fduElt, 0);
		if (fduObj == 0 || fduBuf.state == FduCanceled)
		{
			writeMemo("[?] CFDP unable to cancel inbound FDU.");
			sdr_exit_xn(sdr);
			return 0;
		}

		reqNbr = getReqNbr();
		if (completeInFdu(&fduBuf, fduObj, fduElt, CfdpCancelRequested,
					reqNbr) < 0)
		{
			putErrmsg("CFDP can't cancel inbound transaction.",
					NULL);
			sdr_cancel_xn(sdr);
			return -1;
		}
	}

	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("CFDP can't cancel transaction.", NULL);
		return -1;
	}

	return reqNbr;
}

int	cfdp_suspend(CfdpTransactionId *transactionId)
{
	Sdr	sdr = getIonsdr();
	CfdpDB	*db = getCfdpConstants();
	int	reqNbr;

	CHKERR(transactionId);
	CHKERR(transactionId->sourceEntityNbr.length);
	CHKERR(transactionId->transactionNbr.length);
	if (memcmp(transactionId->sourceEntityNbr.buffer,
			db->ownEntityNbr.buffer, 8) == 0)
	{
		CHKERR(sdr_begin_xn(sdr));
		reqNbr = getReqNbr();
		if (suspendOutFdu(transactionId, CfdpSuspendRequested, reqNbr)
				< 0)
		{
			putErrmsg("CFDP can't suspend outbound transaction.",
					NULL);
			sdr_cancel_xn(sdr);
			return -1;
		}

		if (sdr_end_xn(sdr) < 0)
		{
			putErrmsg("CFDP can't suspend transaction.", NULL);
			return -1;
		}

		return reqNbr;
	}

	/*	Class-1 CFDP can't suspend an inbound transaction.	*/

	return 0;		/*	N/A per 4.1.11.3.1.2.		*/
}

static int	resumeOutFdu(CfdpTransactionId *transactionId)
{
	Sdr		sdr = getIonsdr();
	CfdpVdb		*vdb = getCfdpVdb();
	Object		fduObj;
	OutFdu		fduBuf;
	Object		fduElt;
	CfdpEvent	event;

	CHKERR(sdr_begin_xn(sdr));
	fduObj = findOutFdu(transactionId, &fduBuf, &fduElt);
	if (fduObj == 0 || fduBuf.state != FduSuspended)
	{
		sdr_exit_xn(sdr);
		return 0;
	}

	sdr_stage(sdr, NULL, fduObj, 0);
	fduBuf.state = FduActive;
	sdr_write(sdr, fduObj, (char *) &fduBuf, sizeof(OutFdu));
	memset((char *) &event, 0, sizeof(CfdpEvent));
	event.type = CfdpResumedInd;
	memcpy((char *) &event.transactionId, (char *) transactionId,
			sizeof(CfdpTransactionId));
	event.progress = fduBuf.progress;
	event.reqNbr = getReqNbr();
	if (enqueueCfdpEvent(&event) < 0)
	{
		putErrmsg("CFDP can't report on transaction resumption.", NULL);
		sdr_cancel_xn(sdr);
		return -1;
	}

	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("CFDP can't resume transaction.", NULL);
		return -1;
	}

	sm_SemGive(vdb->fduSemaphore);
	return event.reqNbr;
}

int	cfdp_resume(CfdpTransactionId *transactionId)
{
	CfdpDB	*db = getCfdpConstants();

	CHKERR(transactionId);
	CHKERR(transactionId->sourceEntityNbr.length);
	CHKERR(transactionId->transactionNbr.length);
	if (memcmp(transactionId->sourceEntityNbr.buffer,
			db->ownEntityNbr.buffer, 8) == 0)
	{
		return resumeOutFdu(transactionId);
	}

	/*	Class-1 CFDP can't suspend an inbound transaction.	*/

	return 0;		/*	N/A per 4.1.11.3.1.2.		*/
}

static int	reportOnOutFdu(CfdpTransactionId *transactionId)
{
	Sdr		sdr = getIonsdr();
	Object		fduObj;
	OutFdu		fduBuf;
	Object		fduElt;
	CfdpEvent	event;
	char		reportBuffer[256];

	CHKERR(sdr_begin_xn(sdr));
	fduObj = findOutFdu(transactionId, &fduBuf, &fduElt);
	if (fduObj == 0 || fduBuf.state == FduCanceled)
	{
		sdr_exit_xn(sdr);
		return 0;
	}

	memset((char *) &event, 0, sizeof(CfdpEvent));
	event.type = CfdpReportInd;
	memcpy((char *) &event.transactionId, (char *) transactionId,
			sizeof(CfdpTransactionId));
	isprintf(reportBuffer, sizeof reportBuffer, "state %u  size "
UVAST_FIELDSPEC "  progress " UVAST_FIELDSPEC, (unsigned int) fduBuf.state,
			fduBuf.fileSize, fduBuf.progress);
	event.statusReport = sdr_string_create(sdr, reportBuffer);
	event.reqNbr = getReqNbr();
	if (enqueueCfdpEvent(&event) < 0)
	{
		putErrmsg("CFDP can't report on transaction.", NULL);
		sdr_cancel_xn(sdr);
		return -1;
	}

	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("CFDP can't report on transaction.", NULL);
		return -1;
	}

	return event.reqNbr;
}

static int	reportOnInFdu(CfdpTransactionId *transactionId)
{
	Sdr		sdr = getIonsdr();
	Object		fduObj;
	InFdu		fduBuf;
	Object		fduElt;
	CfdpEvent	event;
	char		reportBuffer[256];

	CHKERR(sdr_begin_xn(sdr));
	fduObj = findInFdu(transactionId, &fduBuf, &fduElt, 0);
	if (fduObj == 0 || fduBuf.state == FduCanceled)
	{
		sdr_exit_xn(sdr);
		return 0;
	}

	memset((char *) &event, 0, sizeof(CfdpEvent));
	event.type = CfdpReportInd;
	memcpy((char *) &event.transactionId, (char *) transactionId,
			sizeof(CfdpTransactionId));
	isprintf(reportBuffer, sizeof reportBuffer, "bytesReceived "
UVAST_FIELDSPEC "  size " UVAST_FIELDSPEC "  progress " UVAST_FIELDSPEC,
			fduBuf.bytesReceived, fduBuf.fileSize, fduBuf.progress);
	event.statusReport = sdr_string_create(sdr, reportBuffer);
	event.reqNbr = getReqNbr();
	if (enqueueCfdpEvent(&event) < 0)
	{
		putErrmsg("CFDP can't report on transaction.", NULL);
		sdr_cancel_xn(sdr);
		return -1;
	}

	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("CFDP can't report on transaction.", NULL);
		return -1;
	}

	return event.reqNbr;
}

int	cfdp_report(CfdpTransactionId *transactionId)
{
	CfdpDB	*db = getCfdpConstants();

	CHKERR(transactionId);
	if (transactionId->sourceEntityNbr.length == 0
	|| transactionId->transactionNbr.length == 0)
	{
		return 0;
	}

	if (memcmp(transactionId->sourceEntityNbr.buffer,
			db->ownEntityNbr.buffer, 8) == 0)
	{
		return reportOnOutFdu(transactionId);
	}
	else
	{
		return reportOnInFdu(transactionId);
	}
}

int	cfdp_get_event(CfdpEventType *type, time_t *time, int *reqNbr,
		CfdpTransactionId *transactionId, char *sourceFileNameBuf,
		char *destFileNameBuf, uvast *fileSize,
		MetadataList *messagesToUser, uvast *offset,
		unsigned int *length, unsigned int *recordBoundsRespected,
		CfdpContinuationState *continuationState,
		unsigned int *segMetadataLength, char *segMetadataBuffer,
		CfdpCondition *condition, uvast *progress,
		CfdpFileStatus *fileStatus, CfdpDeliveryCode *deliveryCode,
		CfdpTransactionId *originatingTransactionId,
		char *statusReportBuf, MetadataList *filestoreResponses)
{
	Sdr		sdr = getIonsdr();
	CfdpVdb		*vdb = getCfdpVdb();
	CfdpDB		*db = getCfdpConstants();
	Object		elt;
	Object		eventAddr;
	CfdpEvent	event;
	Object		msgAddr;
			OBJ_POINTER(MsgToUser, msg);
	char		textBuffer[256];
	char		*content;
	int		len;
	CfdpUserOpsData	opsData;
#ifndef NO_PROXY
	int		proxyPutRequestRecd = 0;
	int		proxyPutCancelRecd = 0;
#endif
	int		originatingTransactionIdRecd = 0;
#ifndef NO_DIRLIST
	int		directoryListingRequestRecd = 0;
#endif
	int		result = 0;

	CHKERR(type);
	CHKERR(time);
	CHKERR(reqNbr);
	CHKERR(transactionId);
	CHKERR(sourceFileNameBuf);
	CHKERR(destFileNameBuf);
	CHKERR(fileSize);
	CHKERR(messagesToUser);
	CHKERR(offset);
	CHKERR(length);
	CHKERR(continuationState);
	CHKERR(segMetadataLength);
	CHKERR(segMetadataBuffer);
	CHKERR(condition);
	CHKERR(progress);
	CHKERR(fileStatus);
	CHKERR(deliveryCode);
	CHKERR(originatingTransactionId);
	CHKERR(statusReportBuf);
	CHKERR(filestoreResponses);
	*type = CfdpNoEvent;		/*	Default.		*/
	CHKERR(sdr_begin_xn(sdr));
	elt = sdr_list_first(sdr, db->events);
	if (elt == 0)
	{
		sdr_exit_xn(sdr);

		/*	Wait until CFDP entity announces an event
		 *	by giving the event semaphore.			*/

		if (sm_SemTake(vdb->eventSemaphore) < 0)
		{
			putErrmsg("CFDP user app can't take semaphore.", NULL);
			return -1;
		}

		if (sm_SemEnded(vdb->eventSemaphore))
		{
			*type = CfdpAccessEnded;
			writeMemo("[i] CFDP user app access terminated.");
			return 0;
		}

		CHKERR(sdr_begin_xn(sdr));
		elt = sdr_list_first(sdr, db->events);
		if (elt == 0)	/*	Function was interrupted.	*/
		{
			sdr_exit_xn(sdr);
			return 0;
		}
	}

	/*	Got next inbound event.  Remove it from the queue.	*/

	eventAddr = sdr_list_data(sdr, elt);
	sdr_list_delete(sdr, elt, (SdrListDeleteFn) NULL, NULL);
	sdr_read(sdr, (char *) &event, eventAddr, sizeof(CfdpEvent));
	sdr_free(sdr, eventAddr);
	*type = event.type;
	*time = event.time;
	*reqNbr = event.reqNbr;
	memcpy((char *) transactionId, (char *) &event.transactionId,
			sizeof(CfdpTransactionId));
	if (event.sourceFileName)
	{
		sdr_string_read(sdr, sourceFileNameBuf, event.sourceFileName);
		sdr_free(sdr, event.sourceFileName);
	}
	else
	{
		*sourceFileNameBuf = '\0';
	}

	if (event.destFileName)
	{
		sdr_string_read(sdr, destFileNameBuf, event.destFileName);
		sdr_free(sdr, event.destFileName);
	}
	else
	{
		*destFileNameBuf = '\0';
	}

	*messagesToUser = event.messagesToUser;
	*fileSize = event.fileSize;
	*offset = event.offset;
	*length = event.length;
	*recordBoundsRespected = event.recordBoundsRespected;
	*continuationState = event.continuationState;
	*segMetadataLength = event.segMetadataLength;
	if (event.segMetadataLength > 0)
	{
		memcpy(segMetadataBuffer, event.segMetadata,
				event.segMetadataLength);
	}

	*condition = event.condition;
	*progress = event.progress;
	*fileStatus = event.fileStatus;
	*deliveryCode = event.deliveryCode;
	memcpy((char *) originatingTransactionId,
			(char *) &event.originatingTransactionId,
			sizeof(CfdpTransactionId));
	if (event.statusReport)
	{
		sdr_string_read(sdr, statusReportBuf, event.statusReport);
		sdr_free(sdr, event.statusReport);
	}
	else
	{
		*statusReportBuf = '\0';
	}

	*filestoreResponses = event.filestoreResponses;
	if (event.messagesToUser == 0)	/*	Can't be std User Ops.	*/
	{
		return sdr_end_xn(sdr);
	}

	/*	This FDU might be a standard User Operations FDU.	*/

	memset((char *) &opsData, 0, sizeof(CfdpUserOpsData));
	for (elt = sdr_list_first(sdr, event.messagesToUser); elt;
			elt = sdr_list_next(sdr, elt))
	{
		msgAddr = sdr_list_data(sdr, elt);
		GET_OBJ_POINTER(sdr, MsgToUser, msg, msgAddr);
		if (msg->length < 5 || msg->text == 0)
		{
			continue;
		}

		sdr_read(sdr, textBuffer, msg->text, msg->length);
		if (strncmp(textBuffer, "cfdp", 4) != 0)
		{
			continue;
		}

		content = textBuffer + 5;
		len = msg->length - 5;
		switch (textBuffer[4])	/*	TLV type.		*/
		{
#ifndef NO_PROXY
		case 0:
			memcpy((char *) &opsData.originatingTransactionId,
					(char *) &event.transactionId,
					sizeof(CfdpTransactionId));
			proxyPutRequestRecd = 1;
			parseProxyPutRequest(content, len, &opsData);
			break;

		case 1:
			parseProxyMsgToUser(content, len, &opsData);
			break;

		case 2:
			parseProxyFilestoreRequest(content, len, &opsData);
			break;

		case 3:
			parseProxyFaultHandlerOverride(content, len,
					&opsData);
			break;

		case 4:
			parseProxyTransmissionMode(content, len, &opsData);
			break;

		case 5:
			parseProxyFlowLabel(content, len, &opsData);
			break;

		case 6:
			parseProxySegmentationControl(content, len,
					&opsData);
			break;

		case 7:
			parseProxyPutResponse(content, len, &opsData);
			break;

		case 8:
			parseProxyFilestoreResponse(content, len, &opsData);
			break;

		case 9:
			proxyPutCancelRecd = 1;
			break;

		case 11:
			parseProxyClosureRequest(content, len, &opsData);
			break;
#endif
		case 10:
			originatingTransactionIdRecd = 1;
			parseOriginatingTransactionId(content, len, &opsData);
			break;
#ifndef NO_DIRLIST
		case 16:
			memcpy((char *) &opsData.originatingTransactionId,
					(char *) &event.transactionId,
					sizeof(CfdpTransactionId));
			directoryListingRequestRecd = 1;
			parseDirectoryListingRequest(content, len, &opsData);
			break;

		case 17:
			parseDirectoryListingResponse(content, len, &opsData);
			break;
#endif
		default:
			break;		/*	Ignore this message.	*/
		}
	}

#ifndef NO_PROXY
	if (proxyPutRequestRecd)
	{
		result = handleProxyPutRequest(&opsData);
	}
	else if (proxyPutCancelRecd)
	{
		result = handleProxyPutCancel(&opsData);
	}
	else
#endif
#ifndef NO_DIRLIST
	if (directoryListingRequestRecd)
	{
		result = handleDirectoryListingRequest(&opsData);
	}
	else
#endif
	if (originatingTransactionIdRecd)
	{
		memcpy((char *) originatingTransactionId,
				(char *) &opsData.originatingTransactionId,
				sizeof(CfdpTransactionId));
	}

	if (result < 0)
	{
		putErrmsg("CFDP failed handling std user operation.", NULL);
		sdr_cancel_xn(sdr);
		return -1;
	}

	if (sdr_end_xn(sdr) < 0)
	{
		putErrmsg("CFDP failed getting event.", NULL);
		return -1;
	}

	return 0;
}

void	cfdp_interrupt()
{
	CfdpVdb	*vdb;

	vdb = getCfdpVdb();
	if (vdb->eventSemaphore != SM_SEM_NONE)
	{
		sm_SemGive(vdb->eventSemaphore);
	}
}

int	cfdp_preview(CfdpTransactionId *transactionId, uvast offset,
		unsigned int length, char *buffer)
{
	Sdr		sdr = getIonsdr();
	CfdpDB		*cfdpdb = getCfdpConstants();
	Object		fduObj;
	InFdu		fduBuf;
	Object		fduElt;
	char		fileName[256];
	int		fd;
	unsigned int	truncatedOffset;
	ssize_t		ret;

	CHKERR(transactionId);
	CHKERR(transactionId->sourceEntityNbr.length);
	CHKERR(transactionId->transactionNbr.length);
	CHKERR(buffer);
	CHKERR(length > 0);
	if (memcmp(transactionId->sourceEntityNbr.buffer,
			cfdpdb->ownEntityNbr.buffer, 8) == 0)
	{
		writeMemo("[?] Previewing outbound transaction.");
		return 0;
	}

	CHKERR(sdr_begin_xn(sdr));
	fduObj = findInFdu(transactionId, &fduBuf, &fduElt, 0);
	if (fduObj == 0 || fduBuf.workingFileName == 0)
	{
		sdr_exit_xn(sdr);
		writeMemo("[?] Can't preview; no such FDU.");
		return 0;
	}

	sdr_string_read(sdr, fileName, fduBuf.workingFileName);
	sdr_exit_xn(sdr);
	fd = ifopen(fileName, O_RDONLY, 0);
	if (fd < 0)
	{
		putSysErrmsg("Can't open working file", fileName);
		return 0;
	}

	if (ilseek(fd, offset, SEEK_SET) < 0)
	{
		close(fd);
		truncatedOffset = offset;
		putSysErrmsg("Can't lseek to offset", utoa(truncatedOffset));
		return -1;
	}

	ret = read(fd, buffer, length);
	if (ret < 0)
	{
		close(fd);
		putSysErrmsg("Can't read from working file", fileName);
		return -1;
	}

	close(fd);
	return (int) ret;
}

int	cfdp_map(CfdpTransactionId *transactionId, unsigned int *extentCount,
		CfdpExtent *extentsArray)
{
	Sdr		sdr = getIonsdr();
	CfdpDB		*cfdpdb = getCfdpConstants();
	Object		fduObj;
	InFdu		fduBuf;
	Object		fduElt;
	Object		elt;
	CfdpExtent	extent;
	unsigned int	i;
	CfdpExtent	*eptr;

	CHKERR(transactionId);
	CHKERR(extentCount);
	CHKERR(extentsArray);
	CHKERR(transactionId->sourceEntityNbr.length > 0);
	CHKERR(transactionId->transactionNbr.length > 0);
	i = *extentCount;	/*	Limit on size of map.		*/
	*extentCount = 0;	/*	Default; nothing mapped.	*/
	if (memcmp(transactionId->sourceEntityNbr.buffer,
			cfdpdb->ownEntityNbr.buffer, 8) == 0)
	{
		writeMemo("[?] Mapping outbound transaction.");
		return 0;
	}

	CHKERR(sdr_begin_xn(sdr));
	fduObj = findInFdu(transactionId, &fduBuf, &fduElt, 0);
	if (fduObj == 0)
	{
		sdr_exit_xn(sdr);
		writeMemo("[?] Can't map; no such FDU.");
		return 0;
	}

	*extentCount = sdr_list_length(sdr, fduBuf.extents);
	elt = sdr_list_first(sdr, fduBuf.extents);
	eptr = extentsArray;
	while (i > 0)
	{
		if (elt == 0)
		{
			break;
		}

		sdr_read(sdr, (char *) &extent, sdr_list_data(sdr, elt),
				sizeof(CfdpExtent));
		eptr->offset = extent.offset;
		eptr->length = extent.length;
		eptr++;
		elt = sdr_list_next(sdr, elt);
		i--;
	}

	sdr_exit_xn(sdr);
	return 0;
}
