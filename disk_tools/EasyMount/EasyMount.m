
/*******************************************************************************

EasyMount - View disk images and archive files as regular folders
Copyright (C) 2013  Alexey Lysiuk

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************/

#import <Cocoa/Cocoa.h>


@interface MountItem : NSObject 
{
@private
	NSString* filePath;
	NSString* mountPath;
	NSString* volumeName;

}

@property(readonly) NSString* filePath;
@property(readonly) NSString* mountPath;
@property(readonly) NSString* volumeName;


- (BOOL)mountFile:(NSString*)filename;

+ (NSDictionary*)extensionToDaemonMap;

@end


@implementation MountItem

@synthesize filePath;
@synthesize mountPath;
@synthesize volumeName;


- (BOOL)mountFile:(NSString*)filename
{
	filePath = filename;

	NSString* daemonPath = [[MountItem extensionToDaemonMap] objectForKey:[filePath pathExtension]];
	if (nil == daemonPath)
	{
		return NO;
	}

	NSFileManager* fileManager = [NSFileManager defaultManager];

	volumeName = [[filePath lastPathComponent] stringByDeletingPathExtension];
	mountPath  = [NSString stringWithFormat:@"/Volumes/%@", volumeName];

	NSString*  nextMountPath = mountPath;
	NSUInteger nextIndex     = 0;

	while ([fileManager fileExistsAtPath:nextMountPath])
	{
		nextMountPath = [mountPath stringByAppendingFormat:@" %u", ++nextIndex];
	}

	if (nextIndex > 0)
	{
		volumeName = [volumeName stringByAppendingFormat:@" %u", nextIndex];
		mountPath = nextMountPath;
	}

	NSString* volumeIcon   = @"/System/Library/Extensions/IOStorageFamily.kext/Contents/Resources/External.icns";
	NSString* mountOptions = [NSString stringWithFormat:@"-olocal,allow_other,ro,volicon=%@,volname=%@", volumeIcon, volumeName];
	NSArray*  arguments    = [NSArray arrayWithObjects:filePath, mountPath, mountOptions, nil];

	[fileManager createDirectoryAtPath:mountPath withIntermediateDirectories:NO attributes:nil error:nil];

	// TODO: check error

	@try
	{
		[NSTask launchedTaskWithLaunchPath:daemonPath arguments:arguments];

		// TODO: open folder in finder
		//[[NSWorkspace sharedWorkspace] openFile:mountPath];
	}
	@catch (NSException *exception)
	{
		(void)exception;

		// TODO: check error

		return NO;
	}

	return YES;
}


+ (NSDictionary*)extensionToDaemonMap
{
	static NSMutableDictionary* result = nil;

	if (nil == result)
	{
		result = [NSMutableDictionary new];

		NSBundle* bundle = [NSBundle mainBundle];
		NSArray* documentTypes = [[bundle infoDictionary] objectForKey:@"CFBundleDocumentTypes"];

		for (NSUInteger i = 0, ei = [documentTypes count]; i < ei; ++i)
		{
			NSDictionary* documentItem = [documentTypes objectAtIndex:i];

			NSArray* extensions = [documentItem objectForKey:@"CFBundleTypeExtensions"];
			NSString* daemonName = [documentItem objectForKey:@"Daemon"];

			for (NSUInteger j = 0, ej = [extensions count]; j < ej; ++j)
			{
				[result setObject:daemonName forKey:[extensions objectAtIndex:j]];
			}
		}
	}

	return result;
}

@end


// -----------------------------------------------------------------------------


@interface ApplicationDelegate : NSResponder
{
@private
	NSMutableArray* mountedItems;
}

- (id)init;

- (BOOL)application:(NSApplication*)theApplication openFile:(NSString*)filename;

@end


@implementation ApplicationDelegate

- (id)init
{
	mountedItems = [NSMutableArray new];

	return self;
}

- (BOOL)application:(NSApplication*)theApplication openFile:(NSString*)filename
{
	(void)theApplication;

	MountItem* mountItem = [MountItem new];
	BOOL result = [mountItem mountFile:filename];

	if (result)
	{
		[mountedItems addObject:mountItem];
	}

	return result;
}

@end


int main(void)
{
	@autoreleasepool
	{
		[NSApplication sharedApplication];

		id applicationDelegate = [ApplicationDelegate new];
		[NSApp setDelegate:applicationDelegate];

		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
		id menubar = [NSMenu new];
		id appMenuItem = [NSMenuItem new];
		[menubar addItem:appMenuItem];
		[NSApp setMainMenu:menubar];
		id appMenu = [NSMenu new];
		id appName = [[NSProcessInfo processInfo] processName];
		id quitTitle = [@"Quit " stringByAppendingString:appName];
		id quitMenuItem = [[NSMenuItem alloc] initWithTitle:quitTitle
													 action:@selector(terminate:) keyEquivalent:@"q"];
		[appMenu addItem:quitMenuItem];
		[appMenuItem setSubmenu:appMenu];

		[NSApp activateIgnoringOtherApps:YES];
		[NSApp run];
	}

	return 0;
}
