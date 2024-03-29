
export type ClrHandle = unknown;
export type BundleHandle = ClrHandle;
export type BundleQueryHandle = ClrHandle;
export type BooleanInt = 1 | 0;
export type DialogResult = DialogResults | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7;
export type MessageBoxButton = MessageBoxButtons | 0 | 1 | 2 | 3;
export type MessageBoxType = MessageBoxTypes | 0 | 1 | 2 | 3;
export type ArrayBufferOrView = ArrayBuffer | ArrayBufferView;

export enum MessageBoxButtons {
    OK = 0,
    OKCancel = 1,
    YesNo = 2,
    YesNoCancel = 3,
}

export enum MessageBoxTypes {
    Information = 0,
    Warning = 1,
    Error = 2,
    Question = 3,
}

export enum DialogResults {
    None = 0,
    Ok = 1,
    Cancel = 2,
    Yes = 3,
    No = 4,
    Abort = 5,
    Ignore = 6,
    Retry = 7,
}

export interface DW2IDERuntimeBindings {
    GetVersion: () => string;
    GetNetVersion: () => string;
    GetGameDirectory: () => string;
    GetUserChosenGameDirectory: () => string;
    ReleaseHandle: (h: ClrHandle) => void;
    HandleToString: (h: ClrHandle) => string;
    LoadBundle: (path: string) => BundleHandle;
    QueryBundleObjects: (h: BundleHandle) => BundleQueryHandle;
    ReadQueriedBundleObject: (obj: BundleQueryHandle) => string | null;
    TryGetObjectId: (objPath: string, objId: ArrayBufferOrView) => BooleanInt;
    GetObjectOffset: (objId: ArrayBufferOrView, wantSourceDirOrBundlePath: boolean, unknown: unknown) => { offset: number, offsetEnd: number, sourceDirOrBundlePath?: string };
    GetObjectType: (objId: ArrayBufferOrView) => string;
    GetObjectSize: (objId: ArrayBufferOrView) => number;
    GetObjectSimplifiedType: (objId: ArrayBufferOrView) => string;
    InstantiateBundleItem: (url: string) => ClrHandle;
    IsImage: (h: ClrHandle) => BooleanInt;
    TryConvertImageToBufferWebp: (h: ClrHandle, mipLevel: number, buffer: ArrayBufferOrView) => BooleanInt;
    TryConvertImageToStreamWebp: (h: ClrHandle, mipLevel: number, onNewBuffer: (buffer: Uint8Array) => void) => BooleanInt;
    GetImageMipLevels: (h: ClrHandle) => number;
    GetImageDimensions: (h: ClrHandle) => number;
    GetImageWidth: (h: ClrHandle, mipLevel:number) => number;
    GetImageHeight: (h: ClrHandle, mipLevel:number) => number;
    GetImageDepth: (h: ClrHandle, mipLevel:number) => number;
    GetImageFormat: (h: ClrHandle) => string;
    GetImageTextureType: (h: ClrHandle) => string;
    TryExportObject: (objId: ArrayBufferOrView, path: string) => BooleanInt;
    TryExportImageAsWebp: (h: ClrHandle, path: string) => BooleanInt;
    TryExportImageAsDds: (h: ClrHandle, path: string) => BooleanInt;
    NewIsolationContext: () => DW2IDERuntimeBindings;
    GetIsolationContextId: () => number;
    GetLastException: () => ClrHandle;
    DebugBreak: () => void;
    ClrDebugBreak: () => void;
    ShowMessageBox: (msg: string, title: string, buttons: MessageBoxButton, type: MessageBoxType) => DialogResult;
    Deisolate: () => void;
}

export const runtimeBindings: DW2IDERuntimeBindings;
export default runtimeBindings;