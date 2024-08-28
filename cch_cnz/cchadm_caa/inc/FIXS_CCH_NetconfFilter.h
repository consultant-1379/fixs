#ifndef FIXS_CCH_NetconfFilter_H
#define	FIXS_CCH_NetconfFilter_H

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_USE


class FIXS_CCH_NetconfFilter {
public:

    FIXS_CCH_NetconfFilter() {

    }

    virtual ~FIXS_CCH_NetconfFilter() {

    }

protected:

    class XStr {
    public:
        // -----------------------------------------------------------------------
        //  Constructors and Destructor
        // -----------------------------------------------------------------------

        XStr(const char* const toTranscode) {
            // Call the private transcoding method
            fUnicodeForm = XMLString::transcode(toTranscode);
        }

        ~XStr() {
            XMLString::release(&fUnicodeForm);
        }

        const XMLCh* unicodeForm() const {
            return fUnicodeForm;
        }

    private:
        XMLCh* fUnicodeForm;
    };

    #define Xi(str) XStr(str).unicodeForm()

};

#endif	/* FIXS_CCH_NetconfFilter_H */

