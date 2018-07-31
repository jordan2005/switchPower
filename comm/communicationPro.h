/*
 * communicationPro.h
 *
 *  Created on: Jul 14, 2018
 *      Author: book
 */

#ifndef COMMUNICATIONPRO_H_
#define COMMUNICATIONPRO_H_
#include "linuxType.h"
#include "debugPrint.h"
#define ID_NAME_BUF {\
						"WRtzdsx", "WRgdfs", "WRspsx", "WRfdbg", "WRfdbd", "WRzkbz", "WRzkbk", "", "", "", \
						"", "", "WRU2qy", "WRI1sx", "WRU2sx", "WRI2sx", "WRsfs", "WRxhfs", "WRjyzdkg", "WRjyfd", \
						"WRI2mcfzsx", "WRI1fzsx", "WRkgpl", "WRUdxs", "WRI1xs", "WRU2xs", "WRI2xs", "WRUdxsjz", "WRI1xsjz", "WRU2xsjz",\
						"WRI2xsjz", "", "", "", "", "", "", "", "", "", \
						"", "", "WRUded", "WRI1ed", "WRU2ed", "WRI2ed", "WRljgw", "WRwxgw", "WRljywsz", "WRwxywsz", \
						"", "", "", "", "", "", "", "", "", "", \
						"WRdy1yxx", "WRdy1gzfs", "WRdy1gzsj", "WRdy1tzsj", "WRdy2yxx", "WRdy2gzfs", "WRdy2gzsj", "WRdy2tzsj", "WRdy3yxx", "WRdy3gzfs", \
						"WRdy3gzsj", "WRdy3tzsj", "WRdy4yxx", "WRdy4gzfs", "WRdy4gzsj", "WRdy4tzsj", "WRdy5yxx", "WRdy5gzfs", "WRdy5gzsj", "WRdy5tzsj", \
						"", "", "", "", "", "", "", "", "", "", \
						"WRyear", "WRmon", "WRday", "WRhour", "WRmin", "WRsec", "", "", "", "", \
						"WRgyqt", "", "WRgyfg", "WRcsqt", "", "", "", "", "", "", \
						"WRdy1cz", "WRdy2cz", "WRdy3cz", "WRdy4cz", "WRdy5cz", "", "", "", "", ""\
					}


int runCommPro(fid* pfdRecv,fid* pfdSend);

#endif /* COMMUNICATIONPRO_H_ */
