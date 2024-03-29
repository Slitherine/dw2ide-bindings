import runtime from '../index.mjs';
import {expect} from 'chai';

describe('dw2ide runtime bindings', () => {
    it('can be imported', () => {
        expect(runtime).to.be.an('object');
        expect(runtime).to.have.property('Initialize');
        expect(runtime['Initialize']).to.be.a('function');
        expect(runtime).to.have.property('GetGameDirectory');
        expect(runtime['GetVersion']).to.be.a('function');
        expect(runtime).to.have.property('GetGameDirectory');
        expect(runtime['GetGameDirectory']).to.be.a('function');
        expect(runtime).to.have.property('GetUserChosenGameDirectory');
        expect(runtime['GetUserChosenGameDirectory']).to.be.a('function');
    });
    it('can be initialized', () => {
        expect(runtime).to.be.an('object');
        expect(runtime).to.have.property('Initialize');
        expect(runtime['Initialize']).to.be.a('function');
        runtime['Initialize']();
    });
    it('can yield the version', () => {
        expect(runtime).to.be.an('object');
        expect(runtime).to.have.property('GetVersion');
        expect(runtime['GetVersion']).to.be.a('function');
        const version = runtime['GetVersion']();
        console.log(`Version: ${version}`);
    });
    it('can yield the .net version', () => {
        expect(runtime).to.be.an('object');
        expect(runtime).to.have.property('GetNetVersion');
        expect(runtime['GetNetVersion']).to.be.a('function');
        const version = runtime['GetNetVersion']();
        console.log(`.NET Version: ${version}`);
    });
    it('can yield the game directory', () => {
        expect(runtime).to.be.an('object');
        expect(runtime).to.have.property('GetGameDirectory');
        expect(runtime['GetGameDirectory']).to.be.a('function');
        const gameDir = runtime['GetGameDirectory']();
        expect(gameDir).to.be.a('string');
        expect(gameDir).to.not.equal('');
        expect(gameDir[gameDir.length - 1]).to.not.equal('\0');
        console.log(`Game directory: ${gameDir}`);
    });
    it('can yield the user-chosen game directory', () => {
        expect(runtime).to.be.an('object');
        expect(runtime).to.have.property('GetUserChosenGameDirectory');
        expect(runtime['GetUserChosenGameDirectory']).to.be.a('function');
        const gameDir = runtime['GetUserChosenGameDirectory']();
        expect(gameDir).to.be.a('string');
        expect(gameDir).to.not.equal('');
        expect(gameDir[gameDir.length - 1]).to.not.equal('\0');
        console.log(`User-chosen game directory: ${gameDir}`);
    });
    it('can load bundle', () => {
        expect(runtime).to.be.an('object');
        expect(runtime).to.have.property('LoadBundle');
        expect(runtime['LoadBundle']).to.be.a('function');
        expect(runtime).to.have.property('GetGameDirectory');
        expect(runtime['GetGameDirectory']).to.be.a('function');
        //runtime['DebugBreak']();
        //runtime['ClrDebugBreak']();
        const gameDir = runtime['GetGameDirectory']();
        expect(gameDir).to.be.a('string');
        expect(gameDir).to.not.equal('');
        expect(gameDir).to.not.include('\0');
        expect(gameDir).to.not.include('\r');
        expect(gameDir).to.not.include('\n');
        expect(gameDir[gameDir.length - 1]).to.not.equal(' ');
        console.log(`Game directory: ${gameDir}`);
        //runtime['ClrDebugBreak']();
        const loaded = runtime['LoadBundle'](gameDir + '\\data\\db\\bundles\\default.bundle');
        expect(loaded).to.be.a('object');
        expect(runtime).to.have.property('QueryBundleObjects');
        expect(runtime['QueryBundleObjects']).to.be.a('function');
        const query = runtime['QueryBundleObjects'](loaded, '*Splash*');
        expect(query).to.be.a('object');
        const first = runtime['ReadQueriedBundleObject'](query);
        expect(first).to.be.a('string');
        expect(first).to.not.equal('');
        expect(first).to.not.include('\0');
        expect(first).to.not.include('\r');
        expect(first).to.not.include('\n');
        expect(first).to.include('Splash');
        expect(first[first.length - 1]).to.not.equal(' ');
        console.log(`First bundle object: ${first}`);
        for (; ;) {
            const item = runtime['ReadQueriedBundleObject'](query);
            if (item === undefined) break;
            expect(item).to.include('Splash');
            console.log(`Next bundle object: ${item}`);
        }
        expect(runtime).to.have.property('HandleToString');
        expect(runtime['HandleToString']).to.be.a('function');
        const name = runtime['HandleToString'](loaded);
        expect(name).to.be.a('string');
        expect(name).to.not.equal('');
        expect(name).to.not.include('\0');
        expect(name).to.not.include('\r');
        expect(name).to.not.include('\n');
        expect(name[name.length - 1]).to.not.equal(' ');
        console.log(`Bundle name: ${name}`);
        expect(name).to.include('default');



    });/*
    it('can show a message box', () => {
        expect(runtime).to.be.an('object');
        expect(runtime).to.have.property('ShowMessageBox');
        expect(runtime['ShowMessageBox']).to.be.a('function');
        expect(runtime).to.have.property('MessageBoxButtons');
        expect(runtime['MessageBoxButtons']).to.be.a('object');
        expect(runtime['MessageBoxButtons']['OK']).to.be.a('number');
        expect(runtime['MessageBoxButtons']['OKCancel']).to.be.a('number');
        expect(runtime['MessageBoxButtons']['YesNo']).to.be.a('number');
        expect(runtime['MessageBoxButtons']['YesNoCancel']).to.be.a('number');
        expect(runtime).to.have.property('MessageBoxType');
        expect(runtime['MessageBoxType']).to.be.a('object');
        expect(runtime['MessageBoxType']['Information']).to.be.a('number');
        expect(runtime['MessageBoxType']['Warning']).to.be.a('number');
        expect(runtime['MessageBoxType']['Error']).to.be.a('number');
        expect(runtime['MessageBoxType']['Question']).to.be.a('number');
        expect(runtime).to.have.property('DialogResult');
        expect(runtime['DialogResult']).to.be.a('object');
        expect(runtime['DialogResult']['None']).to.be.a('number');
        expect(runtime['DialogResult']['Ok']).to.be.a('number');
        expect(runtime['DialogResult']['Cancel']).to.be.a('number');
        expect(runtime['DialogResult']['Yes']).to.be.a('number');
        expect(runtime['DialogResult']['No']).to.be.a('number');
        expect(runtime['DialogResult']['Abort']).to.be.a('number');
        expect(runtime['DialogResult']['Ignore']).to.be.a('number');
        expect(runtime['DialogResult']['Retry']).to.be.a('number');

        //runtime['ClrDebugBreak']();

        const result = runtime['ShowMessageBox']('Hello, world!', 'Hello, world!',
            runtime['MessageBoxButtons']['OK'],
            runtime['MessageBoxType']['Information']);
        expect(result).to.be.a('number');
        expect(result).to.equal(runtime['DialogResult']['Ok']);
    });*/
});