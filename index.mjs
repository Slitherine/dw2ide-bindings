import bindings from 'bindings';

/**
 * @external ClrHandle
 * @description Represents a handle to a managed object; not actually an object, but an 'external' type.
 * @public
 */

/**
 * @typedef {ClrHandle} BundleHandle
 * @public
 */

/**
 * @typedef {ClrHandle} BundleQueryHandle
 * @public
 */

/**
 * @typedef {1|0} BooleanInt
 * @public
 */

/**
 * @typedef {DialogResults|0|1|2|3|4|5|6|7} DialogResult
 * @public
 * @see {DialogResults}
 */

/**
 * @typedef {MessageBoxButtons|0|1|2|3} MessageBoxButton
 * @public
 * @see {MessageBoxButtons}
 */

/**
 * @typedef {MessageBoxTypes|0|1|2|3} MessageBoxType
 * @public
 * @see {MessageBoxTypes}
 */

/**
 * @typedef {ArrayBuffer|ArrayBufferView} ArrayBufferOrView
 * @public
 */

/**
 * @readonly
 * @enum {number}
 * @property {0} OK
 * @property {1} OKCancel
 * @property {2} YesNo
 * @property {3} YesNoCancel
 * @public
 */
export const MessageBoxButtons = {
    OK: 0,
    OKCancel: 1,
    YesNo: 2,
    YesNoCancel: 3,
};

/**
 * @readonly
 * @enum {MessageBoxType}
 * @property {0} Information
 * @property {1} Warning
 * @property {2} Error
 * @property {3} Question
 * @public
 */
export const MessageBoxTypes = {
    Information: 0,
    Warning: 1,
    Error: 2,
    Question: 3,
};

/**
 * @readonly
 * @enum {number}
 * @property {0} None
 * @property {1} Ok
 * @property {2} Cancel
 * @property {3} Yes
 * @property {4} No
 * @property {5} Abort
 * @property {6} Ignore
 * @property {7} Retry
 * @public
 */
export const DialogResults = {
    None: 0,
    Ok: 1,
    Cancel: 2,
    Yes: 3,
    No: 4,
    Abort: 5,
    Ignore: 6,
    Retry: 7,
};
/**
 * @typedef {Object} DW2IDERuntimeBindings
 * @property {()=>string} GetVersion - gets the version of the runtime
 * @property {()=>string} GetNetVersion - gets the version of the .NET runtime
 * @property {()=>string} GetGameDirectory - gets the game directory
 * @property {()=>string} GetUserChosenGameDirectory - gets the user-chosen game directory
 * @property {(h:ClrHandle)=>void} ReleaseHandle - releases a handle
 * @property {(h:ClrHandle)=>string} HandleToString - converts a handle to a string
 * @property {(path:string)=>BundleHandle} LoadBundle - loads a bundle from the specified path
 * @property {(h:BundleHandle)=>BundleQueryHandle} QueryBundleObjects - queries the objects of the bundle
 * @property {(obj:BundleQueryHandle)=>(string|null)} ReadQueriedBundleObject - reads the next object path from the query handle
 * @property {(objPath:string,objId:ArrayBufferOrView)=>BooleanInt} TryGetObjectId - gets the object id of the object (populates the objId buffer, should be at least 16 bytes long)
 * @property {(objId:ArrayBufferOrView,wantSourceDirOrBundlePath:boolean,?)=>{offset:number,offsetEnd:number,sourceDirOrBundlePath?:string}} GetObjectOffset - gets the offset info of the object and optionally the source directory or bundle path
 * @property {(objId:ArrayBufferOrView)=>string} GetObjectType - gets the type of the object
 * @property {(objId:ArrayBufferOrView)=>number} GetObjectSize - gets the size of the object
 * @property {(objId:ArrayBufferOrView)=>string} GetObjectSimplifiedType - gets the simplified type of the object
 * @property {(url:string)=>ClrHandle} InstantiateBundleItem - instantiates a bundle item
 * @property {(h:ClrHandle)=>BooleanInt} IsImage - checks if the handle is an image
 * @property {(h:ClrHandle,mipLevel:number,buffer:ArrayBufferOrView)=>BooleanInt} TryConvertImageToBufferWebp - converts an image to a webp buffer
 * @property {(h:ClrHandle,mipLevel:number,onNewBuffer:(buffer:Uint8Array)=>void)=>BooleanInt} TryConvertImageToStreamWebp - converts an image to a webp stream (via one or more calls to the provided callback)
 * @property {(h:ClrHandle)=>number} GetImageMipLevels - gets the number of mip levels of the image
 * @property {(h:ClrHandle)=>number} GetImageDimensions - gets the number of dimensions of the image
 * @property {(h:ClrHandle)=>number} GetImageWidth - gets the width of the image
 * @property {(h:ClrHandle)=>number} GetImageHeight - gets the height of the image
 * @property {(h:ClrHandle)=>number} GetImageDepth - gets the depth of the image
 * @property {(h:ClrHandle)=>string} GetImageFormat - gets the format of the image
 * @property {(h:ClrHandle)=>string} GetImageTextureType - gets the texture type of the image
 * @property {(objId:ArrayBufferOrView,path:string)=>BooleanInt} TryExportObject - tries to export an object to a file
 * @property {(h:ClrHandle,path:string)=>BooleanInt} TryExportImageAsWebp - tries to export an image as a webp
 * @property {(h:ClrHandle,path:string)=>BooleanInt} TryExportImageAsDds - tries to export an image as a dds
 * @property {()=>DW2IDERuntimeBindings} NewIsolationContext - creates a new isolation context
 * @property {()=>number} GetIsolationContextId - gets the id of the current isolation context
 * @property {()=>ClrHandle} GetLastException - gets the last exception, use with HandleToString and ReleaseHandle
 *
 * @property {()=>void} DebugBreak - invokes the native debugger by triggering a native breakpoint
 * @property {()=>void} ClrDebugBreak - invokes the managed debugger by triggering a managed breakpoint
 * @property {(msg:string,title:string,buttons:MessageBoxButton,type:MessageBoxType)=>DialogResult} ShowMessageBox - shows a message box (this is mainly for GUI debugging purposes)
 * @property {()=>void} Deisolate - deisolates the current context (this is mainly for GUI debugging purposes)
 *
 * @public
 */

/**
 * @module dw2ide_runtime
 * @description The runtime bindings for the DW2IDE project.
 * @type {DW2IDERuntimeBindings}
 */
export const runtimeBindings = bindings('dw2ide_runtime');

export default runtimeBindings;


Object.freeze(MessageBoxButtons);
Object.freeze(MessageBoxTypes);
Object.freeze(DialogResults);