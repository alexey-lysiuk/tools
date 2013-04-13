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
@private
	NSMutableDictionary* extensionToDaemonMap;
}

- (id)init;

- (BOOL)application:(NSApplication*)theApplication openFile:(NSString*)filename;

- (BOOL)mountFile:(NSString*)filename;

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

	return [self mountFile:filename];
}

- (BOOL)mountFile:(NSString*)filename
{
	NSString* daemonPath = [extensionToDaemonMap objectForKey:[filename pathExtension]];

	if (nil == daemonPath)
	{
		return NO;
	}

	NSFileManager* fileManager = [NSFileManager defaultManager];

	NSString* volumeName = [[filename lastPathComponent] stringByDeletingPathExtension];
	NSString* mountPoint = [NSString stringWithFormat:@"/Volumes/%@", volumeName];

	NSString*  nextMountPoint = mountPoint;
	NSUInteger nextIndex = 0;

	while ([fileManager fileExistsAtPath:nextMountPoint])
	{
		nextMountPoint = [mountPoint stringByAppendingFormat:@" %u", ++nextIndex];
	}

	if (nextIndex > 0)
	{
		volumeName = [volumeName stringByAppendingFormat:@" %u", nextIndex];
		mountPoint = nextMountPoint;
	}

	NSString* volumeIcon   = @"/System/Library/Extensions/IOStorageFamily.kext/Contents/Resources/External.icns";
	NSString* mountOptions = [NSString stringWithFormat:@"-olocal,allow_other,ro,volicon=%@,volname=%@", volumeIcon, volumeName];
	NSArray*  arguments    = [NSArray arrayWithObjects:filename, mountPoint, mountOptions, nil];

	[fileManager createDirectoryAtPath:mountPoint withIntermediateDirectories:NO attributes:nil error:nil];

	// TODO: check error

	@try
	{
		[NSTask launchedTaskWithLaunchPath:daemonPath arguments:arguments];

		// TODO: open folder in finder
		//[[NSWorkspace sharedWorkspace] openFile:mountPoint];
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
