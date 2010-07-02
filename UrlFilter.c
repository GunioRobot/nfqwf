#define _GNU_SOURCE 
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "nfq-web-filter-config.h"
#endif

#include <fnmatch.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "Filter.h"
#include "FilterType.h"
#include "HttpReq.h"
#include "nfq_wf_private.h"
#include "HttpConn.h"

/**
* @ingroup FilterObject
* @defgroup UrlFilter URL Filter Object
* @{
*/

struct UrlFilter
{
	FILTER_OBJECT_COMMON
	char *url; /**< URL to  filter */
};

static int UrlFilter_destructor(struct Filter *fobj)
{
	struct UrlFilter *fo = (struct UrlFilter *) fobj; /* filter object */

	if (fo->url) {
		free(fo->url);
		fo->url = NULL;
	}

	return 0;
}

#define URL_STR "url"

static int UrlFilter_load_from_xml(struct Filter *fobj, xmlNode *node)
{
	struct UrlFilter *fo = (struct UrlFilter *) fobj; /* filter object */
	xmlChar *prop = NULL;
	
	DBG(5, "Loading XML config\n");

	prop = xmlGetProp(node, BAD_CAST URL_STR);
	if (!prop) {
		ERROR(" filter/url objects MUST have '%s' XML props \n", URL_STR);
		return -1;
	}

	fo->url = strdup((char*) prop);
	xmlFree(prop);

	if (!fo->url)
		return -1;

	DBG(2, "Loaded URL Filter object ID=%d url='%s'\n",
		Filter_getFilterId(fobj),
		fo->url);

	return 0;
}

static int UrlFilter_matches_req(struct Filter *fobj, struct HttpReq *req)
{
	struct UrlFilter *fo = (struct UrlFilter *) fobj; /* Host filter object */

	DBG(5, "check if req url='%s' contains = '%s'\n", req->url, fo->url);
	if (!req->url) {
		// NOTE can be NULL on 403 cases where we get blocked, with no HTTP request
		return 0;
	}

	if (!fnmatch(fo->url, req->url, FNM_CASEFOLD))
		return 1;

	return 0;
}
static struct Object_ops obj_ops = {
	.obj_type           = "filter/url",
	.obj_size           = sizeof(struct UrlFilter),
};

static struct Filter_ops UrlFilter_obj_ops = {
	.ops                = &obj_ops,
	.foo_destructor     = UrlFilter_destructor,
	.foo_load_from_xml  = UrlFilter_load_from_xml,
#if PRIV_CON_DATA
	.foo_request_start  = UrlFilter_start_req,
#endif	
	.foo_matches_req    = UrlFilter_matches_req,
};


/**
* Initialization function to register this filter type.
*/
static void __init UrlFilter_init(void)
{
	DBG(5, "init URL filter\n");
	FilterType_register(&UrlFilter_obj_ops);
}

/** @} */

