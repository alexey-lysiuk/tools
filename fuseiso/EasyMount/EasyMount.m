//
//  main.m
//  EasyMount
//
//  Created by Alexey on 17.03.13.
//  Copyright (c) 2013 company. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface ApplicationDelegate : NSResponder
{
	NSString* daemonPath;
}

- (id)init;

- (BOOL)application:(NSApplication*)theApplication openFile:(NSString*)filename;

@end

@implementation ApplicationDelegate

- (id)init
{
	daemonPath = [[NSBundle mainBundle] pathForAuxiliaryExecutable:@"fuseiso"];

	return self;
}

- (BOOL)application:(NSApplication*)theApplication openFile:(NSString*)filename
{
	(void)theApplication;

	//[NSAlert alertWithMessageText:nil defaultButton:nil alternateButton:nil otherButton:nil informativeTextWithFormat:@"Filename is %s", [filename UTF8String]];

	@try
	{
		NSString* volumeName = [[filename lastPathComponent] stringByDeletingPathExtension];

		NSString* mountPoint = [NSString stringWithFormat:@"/Volumes/%s", [volumeName UTF8String]];
//		NSString* mountOptions = [NSString stringWithFormat:@"-oallow_other,ro,local,volicon=/System/Library/CoreServices/CoreTypes.bundle/Contents/Resources/CDAudioVolumeIcon.icns,volname=%s", [volumeName UTF8String]];
		NSString* mountOptions = [NSString stringWithFormat:@"-oallow_other,ro,volname=%s", [volumeName UTF8String]];

		NSArray* arguments = [NSArray arrayWithObjects:@"-p", filename, mountPoint, mountOptions, nil];

		[NSTask launchedTaskWithLaunchPath:daemonPath arguments:arguments];
	}
	@catch (NSException *exception)
	{
		(void)exception;

		// TODO ...
	}

	return TRUE;
}

@end

int main(/*int argc, char *argv[]*/ void)
{
	[NSAutoreleasePool new];
	[NSApplication sharedApplication];

	id applicationDelegate = [[ApplicationDelegate new] autorelease];
	[NSApp setDelegate:applicationDelegate];

	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	id menubar = [[NSMenu new] autorelease];
	id appMenuItem = [[NSMenuItem new] autorelease];
	[menubar addItem:appMenuItem];
	[NSApp setMainMenu:menubar];
	id appMenu = [[NSMenu new] autorelease];
	id appName = [[NSProcessInfo processInfo] processName];
	id quitTitle = [@"Quit " stringByAppendingString:appName];
	id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle
												  action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
	[appMenu addItem:quitMenuItem];
	[appMenuItem setSubmenu:appMenu];

	[NSApp activateIgnoringOtherApps:YES];
	[NSApp run];

	return 0;
}






























