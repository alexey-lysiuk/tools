//
//  EasyMount.m
//
// TODO: add license
//
//  Copyright (c) 2013 Alexey Lysiuk
//

#import <Cocoa/Cocoa.h>


@interface ApplicationDelegate : NSResponder
{
	NSMutableDictionary* extensionToDaemonMap;
}

- (id)init;

- (BOOL)application:(NSApplication*)theApplication openFile:(NSString*)filename;

@end


@implementation ApplicationDelegate

- (id)init
{
	extensionToDaemonMap = [[NSMutableDictionary alloc] init];

	NSBundle* bundle = [NSBundle mainBundle];
	NSArray* documentTypes = [[bundle infoDictionary] objectForKey:@"CFBundleDocumentTypes"];

	for (NSUInteger i = 0, ei = [documentTypes count]; i < ei; ++i)
	{
		NSDictionary* documentItem = [documentTypes objectAtIndex:i];

		NSArray* extensions = [documentItem objectForKey:@"CFBundleTypeExtensions"];
		NSString* daemonName = [documentItem objectForKey:@"Daemon"];

		for (NSUInteger j = 0, ej = [extensions count]; j < ej; ++j)
		{
			[extensionToDaemonMap setObject:daemonName forKey:[extensions objectAtIndex:j]];
		}
	}

	return self;
}

- (BOOL)application:(NSApplication*)theApplication openFile:(NSString*)filename
{
	(void)theApplication;

	NSString* daemonPath = [extensionToDaemonMap objectForKey:[filename pathExtension]];

	if (nil == daemonPath)
	{
		return NO;
	}

	NSString* volumeName   = [[filename lastPathComponent] stringByDeletingPathExtension];
	NSString* mountPoint   = [NSString stringWithFormat:@"/Volumes/%s", [volumeName UTF8String]];
	NSString* mountOptions = [NSString stringWithFormat:@"-oallow_other,ro,volname=%s", [volumeName UTF8String]];

	// TODO: mount options
	// -olocal
	// -ovolicon=/System/Library/CoreServices/CoreTypes.bundle/Contents/Resources/CDAudioVolumeIcon.icns

	NSArray* arguments = [NSArray arrayWithObjects:filename, mountPoint, mountOptions, nil];

	[[NSFileManager defaultManager] createDirectoryAtPath:mountPoint withIntermediateDirectories:NO attributes:nil error:nil];

	// TODO: check error

	@try
	{
		[NSTask launchedTaskWithLaunchPath:daemonPath arguments:arguments];
	}
	@catch (NSException *exception)
	{
		(void)exception;

		// TODO: check error
	}

	return TRUE;
}

@end


int main(void)
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
